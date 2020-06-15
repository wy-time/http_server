#ifndef __RIO_H__
#define __RIO_H__
#include <http_server.h> 
#define RIO_BUFSIZE 8192//缓冲区的大小
namespace RIO
{
    struct rio_t
    {
        int rio_fd;//缓冲区对应的文件描述符
        int rio_cnt;//缓冲区中的元素个数
        char * rio_bufptr;//缓冲区头指针
        char rio_buf [RIO_BUFSIZE];//缓冲区
        rio_t (int fd):rio_fd (fd),rio_cnt (0),rio_bufptr(rio_buf){}
    };
    class Rio 
    {
        public:
            //从文件中读取n个字节，存入usrbuf,读到EOF返回0，出错返回-1，正常返回读入的字节数
            ssize_t rio_readn (int fd,void *usrbuf,size_t n);
            //将usrbuf中的n个字节写入文件fd中
            ssize_t rio_writen (int fd,void *usrbuf,size_t n);
            //初始化缓冲区
            void rio_readinitb (rio_t * rp,int fd);
            //从缓冲区读取一行文本,最多读取maxlen的字节，超出部分截断用'\0'结尾
            ssize_t rio_readlineb (rio_t * rp,void * usrbuf,size_t maxlen);
            //从缓冲区读n给字节
            ssize_t rio_readnb (rio_t * rp,void *usrbuf,size_t n);

        private:
            //维护缓冲区，为系统read的带缓冲区版本
            ssize_t rio_read (rio_t *rp,char *usrbuf,size_t);

    };  
}

#endif //__RIO_H__
