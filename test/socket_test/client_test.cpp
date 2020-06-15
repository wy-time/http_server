#include "http_server.h" 
#include "rio.h" 
#include "my_socket.h" 
#define MAXLEN 200
int main ()
{
    char hostname[10]="localhost";
    int clientfd=M_SOCKET::MySocket::openClientfd(hostname,8888);
    RIO::rio_t riobuffer(clientfd);
    char buf[MAXLEN];
    RIO::Rio rio;
    while(fgets(buf,MAXLEN,stdin)!=NULL)
    {
        int len=strlen(buf);
        rio.rio_writen(clientfd,buf,len);
        int n=rio.rio_readlineb(&riobuffer,buf,MAXLEN);
        std::cout<<"client recived "<<n<<" bytes"<<std::endl;
        std::cout<<"the content is "<<buf<<std::endl;
    }
    close(clientfd);
    return 0;
}
