#ifdef __USAGE
%C - dma_tester driver

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
#include <sdma.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#define DCMD_CUSTOM	__DIOT (_DCMD_ELCORE, 228 + 5, int)

char* fw_path = "/tmp/input";

int getbytes(uint8_t* data, const char *filename, uint32_t *size)
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
	
	printf ("bytes read: %d\n",fread(data, 1, *size, f));

	if (data == NULL)
	{
		perror("err reading data");
        return -1;
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

#define DMA_TEST_MEM_SIZE       300

int main( int argc, char** argv )
{
    
    return 0;
    
}
