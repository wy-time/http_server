#include <rio.h>
#include <iostream> 
using namespace std;
int main ()
{
        int fd=open("in",O_RDONLY,S_IROTH);
        RIO::Rio rio;
        RIO::rio_t rp(fd);
        rio.rio_readinitb(&rp,fd);
        char s[100];
        int ret=rio.rio_readlineb (&rp,s,10);
        cout<<ret<<endl;
        cout<<s<<endl;
        close (fd);
}
