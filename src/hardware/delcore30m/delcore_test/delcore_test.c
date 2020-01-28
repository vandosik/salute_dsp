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
#include <elcore-manager.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* fw_path = "/tmp/input";

int getbytes(uint8_t** data, const char *filename, uint32_t *size)
{
	printf("%s: entry\n", __func__);

	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		perror("err opening file");
		*size = 0;
		return -1;
	}
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);

	printf("size of file: %u\n", *size);
	
	*data = malloc(*size + sizeof(elcore_send_t));
	
	if (*data == NULL)
	{
		perror("err alloc data");
	}
	
	printf ("bytes read: %d\n",fread(*data + sizeof(elcore_send_t), 1, *size, f));

	if (*data + sizeof(elcore_send_t) == NULL)
	{
		perror("err reading data");
	}
	
	fclose(f);

	return 0;
}

int writefile(uint8_t *data, const char *filename, uint32_t *size)
{
	printf("%s: entry\n", __func__);

	FILE *f = fopen(filename, "w");
	if (f == NULL) {
		perror("err opening file");
		*size = 0;
		return -1;
	}
	
	
	if (data == NULL)
	{
		perror("No data");
	}
	
	printf ("bytes written: %d\n", fwrite(data, 1, *size, f));

	
	fclose(f);

	return 0;    
}

int main( int argc, char** argv )
{
    int fd;
    int val32 = 0;
    int error;
    
    uint32_t size; //size of program
    uint8_t *fw_data;
    getbytes(&fw_data, fw_path, &size);

    if (fw_data == NULL)
    {
        perror("File opening");
        return -1;
    }
    
    elcore_send_t *send_cfg = (elcore_send_t*)fw_data; 
    
    send_cfg->len = size;
    send_cfg->offset = 0;
    send_cfg->core = 0;
    
    fd = open("/dev/elcore0", O_RDWR);
    
    if ( fd < 0 )
    {
        perror("error opening file");
        return -1;
    }
    
    if (error = devctl( fd, DCMD_ELCORE_SEND, fw_data, size + sizeof(elcore_send_t), NULL ) )
    {
        printf( "DCMD_ELCORE_SEND error: %s\n", strerror ( error ) );
        goto exit;
    }
    
    printf("\n\nProg uploaded\n\n");
    
    if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
        goto exit;
    }
    
    if ( error = devctl( fd, DCMD_ELCORE_START, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_START error: %s\n", strerror ( error ) );
        goto exit;
    }
    
    printf("\n\nProg started\n\n");
    
    delay(5000);
    
    if ( error = devctl( fd, DCMD_ELCORE_STOP, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_STOP error: %s\n", strerror ( error ) );
        goto exit;
    }
    
    printf("\n\nProg stopped\n\n");
    
    if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
        goto exit;
    }
    
    if (error = devctl( fd, DCMD_ELCORE_RECV, fw_data, size + sizeof(elcore_recv_t), NULL ) )
    {
        printf( "DCMD_ELCORE_RECV error: %s\n", strerror ( error ) );
        goto exit;
    }
    
    printf("\n\nProg downloaded\n\n");
    
    writefile(fw_data + sizeof(elcore_recv_t), "/tmp/output", &size);
    
    free(fw_data);
exit:
    close(fd);
    
    return 0;
    
}
