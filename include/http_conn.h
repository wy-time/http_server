#ifndef __HTTP__CONN_H__
#define __HTTP__CONN_H__
#include <http_server.h> 
#include "rio.h" 
#include "my_socket.h" 
#include "http_server.h" 
#include <sstream> 
#define MAXLINE 1024
namespace http_conn
{
    enum HTTP_CODE {NO_REQUEST=0,GET_REQUEST=1,SUCCESS=200,BAD_REQUEST=400,Forbidden=403,NOT_FOUND=404,NOT_IMPLEMENTED=501};//http状态码
    enum READ_LINE_STATUS {LINE_OK=1,LINE_BAD=-1,LINE_OPEN=0};//读取一行的状态，分别为读取成功，读取出错，正在读取
    enum REQUEST_READ_STATUS {REQUEST_LINE,REQUEST_HEADER,REQUEST_BODY};//响应处理到的状态
    class HttpConn
    {
        private:
            RIO::rio_t riobuffer;//读取缓冲区
            RIO::Rio rio;//文件IO类
            HTTP_CODE requeststatuscode;
            READ_LINE_STATUS readlinestatus;
            REQUEST_READ_STATUS requestreadstatus;
            std::string method,uri,version,filename,cgiargs;//请求的方法，uri,http版本,文件名,动态资源的参数
            std::string text;//一行请求信息
            int is_static;//判断是否是一个静态请求
            int listenfd;
            int connfd;//已连接描述符
            READ_LINE_STATUS readLine();//读取一行请求
            HTTP_CODE parse_requestline();//处理请求行
            HTTP_CODE parse_requesthead();//处理请求头
            HTTP_CODE parse_requestbody();//处理请求体
            HTTP_CODE parse_request();//处理http请求
            HTTP_CODE parse_static_request();//处理静态请求
            HTTP_CODE parse_dynamic_request();//处理动态请求
            void parse_error();//处理错误信息
            int parse_uri();//解析请求的资源
        public:
            void init();
    };
}

#endif//__HTTP__CONN_H__
