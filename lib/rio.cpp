#include "rio.h" 
ssize_t RIO:: Rio::rio_readn (int fd,void * usrbuf,size_t n)
{
    size_t isleft=n;
    size_t isread;
    char *buf=static_cast<char*>(usrbuf);
    while (isleft>0)
    {
        if ((isread=read (fd,buf,isleft))<0)
        {
            if (errno==EINTR)
                isread=0;
            else
                return -1;//读取出错
        }else if (isread==0)
            break;
        isleft-=isread;
        buf+=isread;
    }
    return (n-isleft);
}
ssize_t RIO::Rio::rio_writen (int fd,void *usrbuf ,size_t n)
{
    size_t isleft=n;
    size_t iswrite;
    char *buf=static_cast<char*>(usrbuf);
    while (isleft>0)
    {
        if ((iswrite=write(fd,buf,isleft))<=0)
        {
            if (errno==EINTR)
                iswrite=0;
            else
                return -1;//写入出错
        }
        isleft-=iswrite;
        buf+=iswrite;
    }
    return n;
}
void RIO::Rio::rio_readinitb (rio_t *rp,int fd)
{
    rp->rio_fd=fd;
    rp->rio_cnt=0;
    rp->rio_bufptr=rp->rio_buf;
}
ssize_t RIO::Rio::rio_read (rio_t *rp,char *usrbuf,size_t n)
{
    int cnt;
    while (rp->rio_cnt<=0)
    {
        rp->rio_cnt=read (rp->rio_fd,rp->rio_buf,sizeof (rp->rio_buf));
        if (rp->rio_cnt<0)
        {
            if (errno!=EINTR)
                return -1;
        }else if (rp->rio_cnt==0)
                return 0;
        else
            rp->rio_bufptr=rp->rio_buf;
    }
    cnt=std::min (n,(size_t)rp->rio_cnt);
    memcpy (usrbuf,rp->rio_bufptr,cnt);
    rp->rio_cnt-=cnt;
    rp->rio_bufptr+=cnt;
    return cnt;
}
ssize_t RIO::Rio::rio_readlineb (rio_t* rp,void *usrbuf,size_t maxlen)
{
    int isread;
    int i;
    char c,*buf=static_cast<char*> (usrbuf);
    for (i=0;i<maxlen;++i)
    {
        if ((isread=rio_read (rp,&c,1))==1)
        {
            *buf++=c;
            if (c=='\n')
                break;
        }else if (isread==0)
        {
            if (i==0)
                return 0;//EOF,且未读入任何数据
            else
                break;
        }else
            return -1;
    }
    *buf='\0';
    return i+1;
}
ssize_t RIO::Rio::rio_readnb (rio_t* rp,void *usrbuf,size_t n)
{
    int isleft=n;
    int isread;
    char *buf=static_cast<char*> (usrbuf);
    while(isleft>0)
    {
        if ((isread=rio_read(rp,buf,n))<0)
            return -1;
        else if (isread==0)//EOF
            break;
        isleft-=isread;
        buf+=isread;
    }
    return (n-isleft);
}
