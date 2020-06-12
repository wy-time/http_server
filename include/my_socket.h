#ifndef __MY_SOCKET_H__
#define __MY_SOCKET_H__
#include <http_server.h> 
#include <netdb.h> 
//struct in_addr 
//{
//    unsigned int s_addr;//32位的ip地址
//};

//struct sockaddr_in//internet套接字地址
//{
//    unsigned short sin_family;//地址类型总是AF_INET代表因特网地址
//    unsigned short sin_port;//端口
//    in_addr sin_addr;//ip地址
//    unsigned char sin_zero [8];//填充结构体，使得大小和标准格式sockaddr一致
//};   
class MySocket 
{
    public:
        //客户端和服务器简历连接，返回一个描述符，出错返回-1
        static int openClientfd (char *hostname,int port);
        //服务器创建一个监听描述符
        static int openListenfd (int port);
    private:
        MySocket ()=default;
};
#endif//__MY_SOCKET_H__

