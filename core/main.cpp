#include "http_server.h" 
#include "http_conn.h" 
#include "../lib/thread_pool.cpp" 
#include "my_socket.h" 
#define MAX_EVENTNUMBER 10000
void addsig(int sig,void(handler)(int),bool restart = true){//忽略SIGPIPE信号
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler = handler;
    if(restart){
        sa.sa_flags |= SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL) != -1);
}
int main()
{
    addsig(SIGPIPE,SIG_IGN);//忽略SIGPIPE那个信号
    thp::Thread_Pool<http_conn::HttpConn>tp(4);//创建线程池
    int listenfd=M_SOCKET::MySocket::openListenfd(8888);
    int epollfd=epoll_create(256);//创建一个epoll
    http_conn::HttpConn::epollfd=epollfd;
    http_conn::setnoblock(listenfd);//将listenfd设置为非阻塞
    http_conn::addfd(epollfd,listenfd,false,false);//将listenfd放进epoll
    epoll_event events[MAX_EVENTNUMBER];
    std::unordered_map<int,http_conn::HttpConn*> connmap;//将连接指针和描述符映射起来
    while(true)
    {
        int num=epoll_wait(epollfd,events,MAX_EVENTNUMBER,-1);//等待事件
        for(int i=0;i<num;++i)
        {
            int sockfd=events[i].data.fd;
            if(sockfd==listenfd)
            {
                sockaddr_in clientaddr;//客户端套接字地址
                unsigned int clientlen=sizeof(clientaddr);//客户端地址长度
                int connfd=accept(listenfd,(sockaddr*)&clientaddr,&clientlen);
                assert(connfd>=0);
                http_conn::HttpConn *client=new http_conn::HttpConn();
                client->init(connfd);//初始化连接
                connmap.insert(std::make_pair(connfd,client));
            }else if(events[i].events&(EPOLLRDHUP | EPOLLHUP |EPOLLERR))//出现异常
            {
                delete connmap[sockfd];//关闭连接
                connmap.erase(sockfd);
            }else if(events[i].events&EPOLLIN)//读事件
            {
                bool res=connmap[sockfd]->readall();
                if(res)
                    tp.append(connmap[sockfd]);
                else
                {
                    delete connmap[sockfd];
                    connmap.erase(sockfd);
                }
            }else if(events[i].events&EPOLLOUT)//写事件
            {
                if(!connmap[sockfd]->write_back())
                {
                    delete connmap[sockfd];
                    connmap.erase(sockfd);
                }
            }
        }
    }
    return 0;
}
