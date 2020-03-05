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
    int fd;
    int error;
    uint32_t    job_status;
    uint32_t size; //size of program
    uint8_t *src_data;
    uint64_t src_paddr;

#if 0
    uint32_t src_paddr_1;
    printf("input addr:\n");
    scanf("%u", &src_paddr_1);

    printf("got 0x%08x\n", src_paddr_1);
    src_paddr = src_paddr_1;
#endif
    
    fd = open("/dev/elcore0", O_RDWR);
    
    if ( fd < 0 )
    {
        perror("error opening file");
        return -1;
    }
    
	if ((src_data = mmap(NULL, DMA_TEST_MEM_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("SRC mmap err");
		goto exit0;
	}

	if (getbytes(src_data, fw_path, &size) < 0)
    {
		printf("Getbytes error\n");
		goto exit1;
    }

	
    if (mem_offset64(src_data, NOFD, 1, &src_paddr, 0) == -1)
	{
		perror("Get src_phys addr error");
		goto exit1;
	}
    printf("%s: src_phys 0x%08x\n", __func__, src_paddr);
    
    elcore_dmasend_t dma_send = {
        .len = size,
        .offset = 0,
        .core = 0,
        .dma_src = src_paddr
    };
    //BUG: delete this
    sdma_mem_dump(src_data, 40);
    
//     set src data
//     strncpy(src_data, "rRaz raz raz this is hardbass, all v sportivkah adidas. And in niki pazani, listen to \
//     to hardbass basy!!",DMA_TEST_MEM_SIZE-1 );
//     src_data[DMA_TEST_MEM_SIZE-1] = '\0';
   
    if (error = devctl( fd, DCMD_ELCORE_DMASEND, &dma_send, sizeof(elcore_dmasend_t), NULL ) )
    {
        printf( "DCMD_ELCORE_DMASEND error: %s\n", strerror ( error ) );
        goto exit1;
    }
    
    printf("\n\nProg uploaded\n\n");
    
    if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
        goto exit1;
    }
    
    if ( error = devctl( fd, DCMD_ELCORE_START, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_START error: %s\n", strerror ( error ) );
        goto exit1;
    }
    
#if 0
    do {
        job_status = ELCORE_WAIT_NONBLOCK;

        if ( error = devctl( fd, DCMD_ELCORE_JOB_STATUS, &job_status, sizeof(job_status), NULL ) )
        {
            printf( "DCMD_ELCORE_JOB_STATUS error: %s\n", strerror ( error ) );
            goto exit;
        }
        
    } while (job_status == ELCORE_JOB_RUNNING);
#else
//     job_status = ELCORE_WAIT_BLOCK;
// 
//     if ( error = devctl( fd, DCMD_ELCORE_JOB_STATUS, &job_status, sizeof(job_status), NULL ) )
//     {
//         printf( "DCMD_ELCORE_JOB_STATUS error: %s\n", strerror ( error ) );
//         goto exit1;
//     }
#endif

    printf("\n\nProg started\n\n");

    if ( error = devctl( fd, DCMD_ELCORE_STOP, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_STOP error: %s\n", strerror ( error ) );
        goto exit1;
    }
    
    printf("\n\nProg stopped\n\n");
    
    if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
        goto exit1;
    }
    //elcore_dmarecv_t = elcore_dmasend_t
    if (error = devctl( fd, DCMD_ELCORE_DMARECV, &dma_send, sizeof(elcore_dmarecv_t), NULL ) )
    {
        printf( "DCMD_ELCORE_DMARECV error: %s\n", strerror ( error ) );
        goto exit1;
    }
    
    printf("\n\nProg downloaded\n\n");
    sdma_mem_dump(src_data, 40);
    
    
    writefile(src_data, "/tmp/output", &size);
    

    
//     printf("Passed string: \n%s\n", src_data);

    
    
   
    exit1:
        munmap(src_data, SDMA_PROG_MAXSIZE);
    exit0:
        close(fd);
    
    return 0;
    
}