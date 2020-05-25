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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#define DCMD_CUSTOM	__DIOT (_DCMD_ELCORE, 228 + 5, int)

char* fw_path_1 = "/tmp/detector";

int mem_dump(uint8_t* addr, uint32_t len)
{
	printf("%s: entry\n", __func__);
	uint32_t iter = 0;
	
	printf("Numeric\n");
	for (; iter < len; iter++)
	{
		if (iter % 16 == 0)
		{
			printf("\n");
		}
		printf(" %02x ", *(addr+iter));

	}
	
// 	printf("Symbolic\n");
// 	for (iter = 0; iter < len; iter++)
// 	{
// 		if (iter % 16 == 0)
// 		{
// 			printf("\n");
// 		}
// 		printf(" %c ", *(addr+iter));
// 	}
	
	printf("\n");
    
	return 0;
}

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
    uint32_t size_1, size_2; //size of program
    uint8_t *src_data_1, *arg_data;
    uint64_t src_paddr_1, arg_paddr;
   

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
    
    
    
//     if ((pram_addr = mmap(NULL, DMA_TEST_MEM_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE, 0, fd, 0)) == MAP_FAILED )
//     {
//         perror("DSP mmap error");
//     }
//     
    
	if ((src_data_1 = mmap(NULL, DMA_TEST_MEM_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("SRC mmap err");
		goto exit0;
	}
	
	size_2 = sizeof(uint32_t) * 2;
	
    if ((arg_data = mmap(NULL, size_2 , PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("SRC mmap err");
		goto exit1;
	}

	if (getbytes(src_data_1, fw_path_1, &size_1) < 0)
    {
		printf("Getbytes error\n");
		goto exit2;
    }

    
    
//     if (getbytes(src_data_2, fw_path_2, &size_2) < 0)
//     {
// 		printf("Getbytes error\n");
// 		goto exit2;
//     }
	
    if (mem_offset64(src_data_1, NOFD, 1, &src_paddr_1, 0) == -1)
	{
		perror("Get src_phys addr error");
		goto exit2;
	}
    printf("%s: src_phys_1 0x%08x\n", __func__, src_paddr_1);
   
    if (mem_offset64(arg_data, NOFD, 1, &arg_paddr, 0) == -1)
	{
		perror("Get src_phys addr error");
		goto exit2;
	}
    printf("%s: src_phys_2 0x%08x\n", __func__, src_paddr_1);
    
    (*(uint32_t*)arg_data) = 0x12345678;
    (*(uint32_t*)(arg_data + 4)) = 0x11111111;
    
    ELCORE_JOB firs_job = {
        .core = 1,
        .inum = 1,
        .input[0] = {size_2, arg_paddr},
        .onum = 1,
        .output[0] = {size_2, arg_paddr},
        .code = {size_1, src_paddr_1}
    };
   
//     ELCORE_JOB second_job = {
//         .core = 0,
//         .inum = 0,
//         .onum = 0,
//         .code = {size_2, src_paddr_2}
//     };
   
    if (error = devctl( fd, DCMD_ELCORE_JOB_CREATE, &firs_job, sizeof(ELCORE_JOB), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_CREATE error: %s\n", strerror ( error ) );
        goto exit2;
    }
    
    printf("\n\nJob uploaded job id: %u\n\n", firs_job.id);
    
//     if (error = devctl( fd, DCMD_ELCORE_JOB_CREATE, &second_job, sizeof(ELCORE_JOB), NULL ) )
//     {
//         printf( "DCMD_ELCORE_JOB_CREATE error: %s\n", strerror ( error ) );
//         goto exit2;
//     }
//     
//     printf("\n\nJob uploaded job id: %u\n\n", second_job.id);
//     
    
    if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
        goto exit2;
    }
    
    
    if ( error = devctl( fd, DCMD_ELCORE_JOB_ENQUEUE, &firs_job.id, sizeof(firs_job.id), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_ENQUEUE error: %s\n", strerror ( error ) );
        goto exit2;
    }
   
//    if ( error = devctl( fd, DCMD_ELCORE_JOB_ENQUEUE, &second_job.id, sizeof(second_job.id), NULL ) )
//     {
//         printf( "DCMD_ELCORE_JOB_ENQUEUE error: %s\n", strerror ( error ) );
//         goto exit2;
//     }

    printf("\n\nProg started\n\n");
    
    job_status = firs_job.id;
    
    if ( error = devctl( fd, DCMD_ELCORE_JOB_WAIT, &job_status, sizeof(job_status), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_STATUS error: %s\n", strerror ( error ) );
        goto exit2;
    }

    printf("job_1 rc: %d\n", job_status);
    
//     job_status = second_job.id;
//     
//     if ( error = devctl( fd, DCMD_ELCORE_JOB_WAIT, &job_status, sizeof(job_status), NULL ) )
//     {
//         printf( "DCMD_ELCORE_JOB_STATUS error: %s\n", strerror ( error ) );
//         goto exit2;
//     }
// 
//     printf("job_2 rc: %d\n", job_status);
    
    
    
    if ( error = devctl( fd, DCMD_ELCORE_STOP, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_STOP error: %s\n", strerror ( error ) );
        goto exit2;
    }
    
    printf("\n\nProg stopped\n\n");
    
    if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
        goto exit2;
    }
    
    //elcore_dmarecv_t = elcore_dmasend_t
    if (error = devctl( fd, DCMD_ELCORE_JOB_RESULTS, &firs_job.id, sizeof(firs_job.id), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_RESULTS error: %s\n", strerror ( error ) );
        goto exit2;
    }
//     
//     if (error = devctl( fd, DCMD_ELCORE_JOB_RESULTS, &second_job.id, sizeof(second_job.id), NULL ) )
//     {
//         printf( "DCMD_ELCORE_JOB_RESULTS error: %s\n", strerror ( error ) );
//         goto exit2;
//     }
    
    printf("\n\nProg downloaded\n\n");
    
    
    writefile(src_data_1, "/tmp/output1", &size_1);
//     writefile(arg_data, "/tmp/output2", &size_2);
    
	uint32_t iter = 0;
	
    
    mem_dump(src_data_1, size_1);
    mem_dump(arg_data, size_2);
   
    
    
    if (error = devctl( fd, DCMD_ELCORE_JOB_RELEASE, &firs_job.id, sizeof(firs_job.id), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_RELEASE error: %s\n", strerror ( error ) );
        goto exit1;
    }
    
//     if (error = devctl( fd, DCMD_ELCORE_JOB_RELEASE, &second_job.id, sizeof(second_job.id), NULL ) )
//     {
//         printf( "DCMD_ELCORE_JOB_RELEASE error: %s\n", strerror ( error ) );
//         goto exit1;
//     }
    
//     printf("Passed string: \n%s\n", src_data);

    
    
    exit2:
        munmap(arg_data, DMA_TEST_MEM_SIZE);
    exit1:
        munmap(src_data_1, DMA_TEST_MEM_SIZE);
    exit0:
        close(fd);
    
    return 0;
    
}
