#ifdef __USAGE
%C - SPI driver

Syntax:
%C [-u unit]

Options:
-u unit    Set spi unit number (default: 0).
-d         driver module name
#endif

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdint.h>
#include <unistd.h>
#include <devctl.h>
#include <elcore-master.h>


int main( int argc, char** argv )
{
    int fd;
    
    
    
    
    fd = open("/dev/elcore0", O_RDWR);
    
    if ( fd == -1 )
        return -1;
    
    
    
    close(fd);
    
    return 0;
    
}
