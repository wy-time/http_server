#include "http_conn.h" 
int http_conn::HttpConn::epollfd=-1;
void http_conn::setnoblock(int fd)//设置文件描述符非阻塞
{
    int flag=fcntl(fd,F_GETFL);
    assert(flag>0);
    fcntl(fd,F_SETFL,flag|O_NONBLOCK);
}

void http_conn::addfd(int epollfd,int fd,bool et,bool oneshot)//将文件描述符添加进epoll进行监听
{
    epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLIN |EPOLLRDHUP;//注册两个事件
    if(et)
        event.events|=EPOLLET;//边沿触发模式
    if(oneshot)
        event.events|=EPOLLONESHOT;//只响应一次
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);//注册事件
}

void http_conn::modfd(int epollfd,int fd,int ev)//重置事件，可以接受新的事件
{
    epoll_event event;
    event.data.fd=fd;
    event.events=ev | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
    epoll_ctl(epollfd,EPOLL_CTL_MOD,fd,&event);//注册事件
}

void http_conn::removefd(int epollfd,int fd)//将描述符移除出epool,并关闭文件描述符
{
    epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
    close(fd);
}
void http_conn::HttpConn::init(int fd)
{
    requestreadstatus=REQUEST_LINE;//初始化读取状态为读取请求行
    requeststatuscode=NO_REQUEST;//初始化http请求的状态
    connfd=fd;//给已连接描述符赋值
    rio.rio_readinitb(&riobuffer,connfd);//将已连接描述符和读缓冲区绑定
    r_check_idx=0;
    w_send_idx=0;
    text.clear();
    readbuffer.clear();
    writebuffer.clear();
    setnoblock(connfd);
    addfd(epollfd,connfd,true,true);//将该连接加入监听
}

bool http_conn::HttpConn::readall()//读取所有数据放入读缓冲区
{
    int isread;
    char c;
    //循环读取数据直到无数据可读或客户端关闭连接
    while(true)
    {
        isread=rio.rio_readnb(&riobuffer,&c,1);
        if(isread>0)
            readbuffer.append(1,c);
        else if(isread==-1)
        {
            if(errno == EAGAIN || errno == EWOULDBLOCK)
                break;
            return false;
        }else if(isread==0)
            return false;
    }
    return true;
}
http_conn::READ_LINE_STATUS http_conn::HttpConn::readLine()//读取一行请求(从读取缓冲区)
{
    if(readlinestatus!=LINE_OPEN)//没读取到完整行的时候不清空
        text.clear();
    char c;
    http_conn::READ_LINE_STATUS statu=LINE_OPEN;
    int len=readbuffer.length();
    for(;r_check_idx<len;r_check_idx++)
    {
        c=readbuffer[r_check_idx];
        if (c=='\n')
        {
            if (r_check_idx-1>=0&&readbuffer[r_check_idx-1]=='\r')//\n前面是\r
            {
                statu=LINE_OK;
                text.append(1,c);
                break;
            }
            else//\n前面不是\r，则出错
            {
                statu=LINE_BAD;//出现单独的'\n'
                break;
            }
        }else if(c!='\r')
        {
            if(r_check_idx-1>=0&&readbuffer[r_check_idx-1]=='\r')//\r后面不是\n则出错
            {
                statu=LINE_BAD;
                break;
            }
        }
        text.append(1,c);
    }
    r_check_idx++;
    return statu;
}
http_conn::HTTP_CODE http_conn::HttpConn::parse_requestline()//处理请求行
{
    if (text.empty())
        return NO_REQUEST;
    std::stringstream ss(text);
    ss>>method>>uri>>version;
    transform(method.begin(), method.end(), method.begin(), ::toupper);
    if (method.compare("GET")!=0/*&&method.compare("POST")!=0*/)
    {
        return NOT_IMPLEMENTED;
    }
    is_static=parse_uri();
    requestreadstatus=REQUEST_HEADER;
    return SUCCESS;
}
http_conn::HTTP_CODE http_conn::HttpConn::parse_requesthead()//处理请求头
{
    if(text=="\r\n")
    {
        requestreadstatus=REQUEST_BODY;
        if (method=="GET")
            return GET_REQUEST;    
    }
    return SUCCESS;
}
http_conn::HTTP_CODE http_conn::HttpConn::parse_requestbody()//处理请求体
{
    if (text.empty())
        return GET_REQUEST;
    else
    {
        
        return SUCCESS;
    }
}

http_conn::HTTP_CODE http_conn::HttpConn::parse_request()//处理http请求
{
    while((readlinestatus=readLine())==LINE_OK)//成功读取到一行
    {
        switch(requestreadstatus)
        {
            case REQUEST_LINE:
            {
                HTTP_CODE ret=parse_requestline();
                if(ret!=SUCCESS)
                {
                    return ret;
                }
                break;
            }
            case REQUEST_HEADER:
            {
                HTTP_CODE ret=parse_requesthead();
                if (ret==BAD_REQUEST)
                    return BAD_REQUEST;
                else if (ret==GET_REQUEST)
                {
                    if(is_static)
                        return parse_static_request();
                    else
                        return parse_dynamic_request();
                }
                break;
            }
            case REQUEST_BODY:
                HTTP_CODE ret=parse_requestbody();
                if (ret==BAD_REQUEST)
                    return BAD_REQUEST;
                else if(ret==GET_REQUEST)
                {
                    if(is_static)
                        return parse_static_request();
                    else
                        return parse_dynamic_request();
                }
                break;

        }
    }
    if(readlinestatus==LINE_BAD)
        return BAD_REQUEST;
    return NO_REQUEST;
}

int http_conn::HttpConn::parse_uri()//解析请求的资源
{
    if(uri.find("cgi-bin")==std::string::npos)//静态资源
    {
        cgiargs="";//没有参数
        filename="/home/time/work/http_server/bin/static";//默认网站根目录味当前目录
        filename+=uri;//加上请求的目录
        if(*uri.rbegin()=='/')
            filename+="index.html";//加上默认访问页面
        return 1;//是静态请求
    }else
    {
        int pos=uri.find("?");
        if(pos!=std::string::npos)//有程序参数
        {
            cgiargs=uri.substr(pos+1);
            uri=uri.substr(0,pos);
        }else
            cgiargs="";
        filename=".";
        filename+=uri;
        return 0;//是动态请求
    }
}
http_conn::HTTP_CODE http_conn::HttpConn::parse_static_request()//处理静态请求
{
    struct stat sbuf;
    std::string filetype;
    if(stat(filename.c_str(),&sbuf)<0)//获取文件的信息
    {
        return NOT_FOUND;
    }
    if(!(S_ISREG(sbuf.st_mode))||!(S_IRUSR&sbuf.st_mode))//没读取权限或不是一般文件
    {
        return Forbidden;
    }
    if(filename.find(".html")!=std::string::npos)
        filetype="text/html";
    else if(filename.find(".gif"))
        filetype="image/gif";
    else if(filename.find(".jpg"))
        filetype="image/jpeg";
    else
        filetype="text/plain";
    std::stringstream ss;
    ss<<"HTTP/1.0 200 OK\r\n";
    ss<<"Server: Time Web Server\r\n";
    ss<<"Content-length: "<<sbuf.st_size<<"\r\n";
    ss<<"Content-type: "<<filetype<<"\r\n\r\n";
    int filefd=open(filename.c_str(),O_RDONLY,0);
    char *usrbuf=(char*)malloc(sbuf.st_size+2);
    rio.rio_readn(filefd,usrbuf,sbuf.st_size);
    writebuffer=ss.str()+usrbuf;//将响应行和响应头,还有响应体写入写缓冲区
    free(usrbuf);
    close(filefd);
    return SUCCESS;
}

http_conn::HTTP_CODE http_conn::HttpConn::parse_dynamic_request()//处理动态请求
{
    return SUCCESS;
}

void http_conn::HttpConn::parse_error()//处理错误信息
{
    std::stringstream ss;
    std::string errnum;std::string errmsg,longmsg;
    switch(requeststatuscode)
    {
        case SUCCESS:
        case NO_REQUEST:
        case GET_REQUEST:
            break;
        case BAD_REQUEST:
            errnum="400";
            errmsg="Bad Request";
            longmsg="The request could not be understood by the server due to malformed syntax";
            break;
        case Forbidden:
            errnum="403";
            errmsg="Forbidden";
            longmsg="The server understood the request, but is refusing to fulfill it";
            break;
        case NOT_FOUND:
            errnum="404";
            errmsg="NOT_FOUND";
            longmsg="The server has not found anything matching the Request-URI";
            break;
        case NOT_IMPLEMENTED:
            errnum="501";
            errmsg="NOT_IMPLEMENTED";
            longmsg="The server does not support the functionality required to fulfill the request";
            break;
    }
    //构造响应
    std::string body="<html><title>Server Error</title>";
    body+="<body bgcolor=""ffffff"">\r\n";
    body+=errnum+": "+errmsg+"\r\n";
    body+="<p>"+longmsg+": "+filename+"\r\n";
    body+="<hr><em>The Time Web Server</em>\r\n";
    ss<<"HTTP/1.0 "<<errnum<<" "<<errmsg<<"\r\n";
    ss<<"Content-type: "<<"text/html\r\n";
    ss<<"Content-length: "<<body.size()<<"\r\n\r\n";
    ss<<body;
    writebuffer=ss.str();//将响应放入写缓冲区
}

bool http_conn::HttpConn::write_back()//将响应写入文件
{
    //因为是非阻塞且为ET触发，所以需要一次把数据发完
    int need_to_send=writebuffer.length();
    while(w_send_idx<need_to_send-1)
    {
        int issend=rio.rio_writen(connfd,((char *) writebuffer.c_str())+w_send_idx,need_to_send-w_send_idx);
        if(issend<0)
        {
            if(errno==EAGAIN)//暂时不可写
            {
                modfd(epollfd,connfd,EPOLLOUT);//等待下一次输出
                return true;
            }else
                return false;
        }
        w_send_idx=issend-1;
    }
    return false;
}
void http_conn::HttpConn::run()
{
    requeststatuscode=parse_request();//解析http请求
    if(requeststatuscode==NO_REQUEST)
    {
        modfd(epollfd,connfd,EPOLLIN);//重置事件等待下一次读取
        return ;
    }else if(requeststatuscode!=SUCCESS)
    {
        parse_error();
    }
    modfd(epollfd,connfd,EPOLLOUT);//重置事件等待下一次输出
}
http_conn::HttpConn::~HttpConn()//析构函数，关闭已连接描述符
{
    removefd(epollfd,connfd);
}

