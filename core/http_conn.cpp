#include "http_conn.h" 

void http_conn::HttpConn::init()
{
    requestreadstatus=REQUEST_LINE;
    requeststatuscode=NO_REQUEST;

    listenfd=M_SOCKET::MySocket::openListenfd(8888);//在8888端口打开一个监听描述符
    struct sockaddr_in clientaddr;
    unsigned int clientlen=sizeof(clientaddr);
    connfd=accept (listenfd, (sockaddr*) &clientaddr,&clientlen);
    rio.rio_readinitb(&riobuffer,connfd);
    requeststatuscode=parse_request();
    if(requeststatuscode!=SUCCESS)
    {
        parse_error();
    }
    close(connfd);
}

http_conn::READ_LINE_STATUS http_conn::HttpConn::readLine()//读取一行请求
{
    text.clear();
    char c;
    http_conn::READ_LINE_STATUS statu=LINE_OPEN;
    while(statu==LINE_OPEN)
    {
        int isread=rio.rio_readnb(&riobuffer,&c,1);
        switch (isread)
        {
            case -1:
                statu=LINE_BAD;
                break;
            case 1:
                statu=LINE_OPEN;
                if (c=='\n')
                {
                    if (*text.rbegin()=='\r')
                        statu=LINE_OK;
                    else
                        statu=LINE_BAD;//出现单独的'\n'
                }else
                {
                    if(*text.rend()=='\r')
                        statu=LINE_BAD;//出现单独的'\r'
                }
                text.append(1,c);
                break;
        }
    }
    return statu;
}
http_conn::HTTP_CODE http_conn::HttpConn::parse_requestline()//处理请求行
{
    if (text.empty())
        return NO_REQUEST;
    std::stringstream ss(text);
    ss>>method>>uri>>version;
    transform(method.begin(), method.end(), method.begin(), ::toupper);
    if (method.compare("GET")!=0&&method.compare("POST")!=0)
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
    return BAD_REQUEST;
}

int http_conn::HttpConn::parse_uri()//解析请求的资源
{
    if(uri.find("cgi-bin")==std::string::npos)//静态资源
    {
        cgiargs="";//没有参数
        filename="./static";//默认网站根目录味当前目录
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
    std::string filetype,buf;
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
    buf=ss.str();
    rio.rio_writen(connfd,(char*)buf.c_str(),sbuf.st_size);
    int filefd=open(filename.c_str(),O_RDONLY,0);
    char *usrbuf=(char*)malloc(sbuf.st_size+2);
    RIO::rio_t readbuf(filefd);
    rio.rio_readn(filefd,usrbuf,sbuf.st_size);
    rio.rio_writen(connfd,usrbuf,sbuf.st_size);
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
    body=ss.str();
    rio.rio_writen(connfd,(char *)body.c_str(),body.size());
}
