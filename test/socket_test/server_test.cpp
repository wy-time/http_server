#include "rio.h"
#include "my_socket.h" 
#include <iostream> 
#define MAXLEN 200
int main ()
{
    int listenfd=M_SOCKET::MySocket::openListenfd (8888);//监听描述符
    sockaddr_in clientaddr;//客户端套接字地址
    unsigned int clientlen;//客户端地址长度
    hostent *client; //客户端信息
    memset(&clientaddr,0,sizeof(clientaddr));
    RIO::rio_t riobuffer(0);
    while(true)
    {
        clientlen=sizeof(clientaddr);
        int connectfd=accept(listenfd,(sockaddr*)&clientaddr,&clientlen);
        client=gethostbyaddr((const char *)&(clientaddr.sin_addr.s_addr),sizeof(clientaddr.sin_addr.s_addr),0);//获取客户端的信息
        char *claddr=inet_ntoa(clientaddr.sin_addr);
        std::cout<<"server connect to "<<client->h_name<<"("<<claddr<<")"<<std::endl;
        //printf("server connect to %s (%s)\n",client->h_name,claddr);
        RIO::Rio rio;
        rio.rio_readinitb(&riobuffer,connectfd);
        char buf[MAXLEN];
        int n;
        while((n=rio.rio_readlineb(&riobuffer,buf,MAXLEN))>0)
        {
            std::cout<<"server received "<<n<<" bytes"<<std::endl;
            std::cout<<"the context is "<<buf<<std::endl;
            //printf("the context is %s\n",buf);
            rio.rio_writen(connectfd,buf,n);//写回去
        }
        close(connectfd);
    }
    return 0;
}
