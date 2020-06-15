#ifndef __HTTP__CONN_H__
#define __HTTP__CONN_H__
#include <http_server.h> 
namespace http_conn
{
    enum HTTP_CODE {SUCCESS=200,MOVED=301,BAD_REQUEST=400,Forbidden=403,NOT_FOUND=404,NOT_IMPLEMENTED=501};//http状态码
    enum READ_LINE_STATUS {LINE_OK=1,LINE_BAD=-1,LINE_OPEN=0};//读取一行的状态，分别为读取成功，读取出错，正在读取
    enum REQUEST_READ_STATUS {REQUEST_LINE,REQUEST_HEADER,REQUEST_BODY};//响应处理到的状态
    class HttpConn
    {
        private:
            HTTP_CODE requeststatuscode;
            READ_LINE_STATUS readlinestatus;
            REQUEST_READ_STATUS requestreadstatus;
            std::string text;
            int listenfd;
            READ_LINE_STATUS readLine();
            void init(int listenfd);
        public:
            HttpConn(int listenfd)
            {
                init(listenfd);
            }

    };
}

#endif//__HTTP__CONN_H__
