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
#include <errno.h>
#include <elcore-master.h>

#include <stdio.h>
#include <string.h>

int main( int argc, char** argv )
{
    int fd;
    int val32 = 0;
    int error;
    
    
    
    
    fd = open("/dev/elcore0", O_RDWR);
    
    if ( fd < 0 )
    {
        perror("error opening file");
        return -1;
    }
    
    if (error = devctl( fd, DCMD_ELCORE_T, &val32, sizeof(val32), NULL ) )
    {
        printf( "DCMD_ELCORE_T error: %s\n", strerror ( error ) );
        goto exit;
    }
    
    if ( error = devctl( fd, DCMD_ELCORE_F, &val32, sizeof(val32), NULL ) )
    {
        printf( "DCMD_ELCORE_F error: %s\n", strerror ( error ) );
        goto exit;
    }
    
    if ( error = devctl( fd, DCMD_ELCORE_N, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_N error: %s\n", strerror ( error ) );
        goto exit;
    }
    
    if ( error = devctl( fd, DCMD_ELCORE_TF, &val32, sizeof(val32), NULL ) )
    {
        printf( "DCMD_ELCORE_TF error: %s\n", strerror ( error ) );
        goto exit;
    }
    
exit:
    close(fd);
    
    return 0;
    
}
