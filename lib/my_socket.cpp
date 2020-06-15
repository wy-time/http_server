#include "my_socket.h" 
#define LISTENQ 1024
int M_SOCKET::MySocket::openClientfd(char* hostname,int port)
{
    int clientfd;
    sockaddr_in serveraddr;//套接字地址
    hostent *host;//DNS主机条目
    if ((clientfd=socket (AF_INET,SOCK_STREAM,0))<0)
        return -1;//出错，错误代码是是errno
    if ((host=gethostbyname(hostname))==NULL)
        return -2;//出错错误代码是h_error
    //给套接字地址赋值
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(port);
    memcpy(&serveraddr.sin_addr.s_addr,host->h_addr_list[0],host->h_length);
    //向服务端发起连接
    if (connect(clientfd,(sockaddr*)&serveraddr,sizeof(serveraddr))<0)
        return -1;
    return clientfd;
}

int M_SOCKET::MySocket::openListenfd(int port)
{
    int listenfd,optval=1;
    sockaddr_in serveraddr;
    if ((listenfd=socket(AF_INET,SOCK_STREAM,0))<0)
        return -1;
    if (setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(const void *)&optval,sizeof(int))<0)
        return -1;
    memset(&serveraddr,0,sizeof(serveraddr));
    serveraddr.sin_family=AF_INET;
    serveraddr.sin_port=htons(port);
    serveraddr.sin_addr.s_addr=htonl(INADDR_ANY);
    if(bind(listenfd,(sockaddr*)&serveraddr,sizeof(serveraddr))<0)
        return -1;
    if(listen(listenfd,LISTENQ)<0)
        return -1;
    return listenfd;
}

