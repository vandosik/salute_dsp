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
#include <pthread.h>
#include <devctl.h>
#include <errno.h>
#include <elcore-manager.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>


char* fw_path = "/tmp/sum";
int fd;

int mem_dump(uint8_t* addr, uint32_t len)
{
	printf("%s: entry\n", __func__);
	uint32_t iter = 0;
	
	for (; iter < len; iter++)
	{
		if (iter % 8 == 0)
		{
			printf("\n");
		}
		printf(" %02x ", *(addr+iter));

	}
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



#define DSP_PROG_MAX_SIZE       300
int DSP_JOBS_COUNT         = 6;
int DSP_ITERATIONS         = 1; 
uint8_t *arg_data;


void *work_thread(void *job)
{
    ELCORE_JOB* job_ptr = job;
    ELCORE_JOB cur_job = {
        .core = job_ptr->core,
        .inum = job_ptr->inum,
        .onum = job_ptr->onum,
        .code = job_ptr->code,
        .input[0] = {job_ptr->input[0].size, job_ptr->input[0].client_paddr},
        .output[0] = {job_ptr->output[0].size, job_ptr->output[0].client_paddr}
    };
    int error;
	int thr_it;
	
	for (thr_it = 0; thr_it < DSP_ITERATIONS; thr_it++)
	{
		

		if (error = devctl( fd, DCMD_ELCORE_JOB_CREATE, &cur_job, sizeof(ELCORE_JOB), NULL ) )
		{
		    printf( "DCMD_ELCORE_JOB_CREATE error: %s\n", strerror ( error ) );
		    return NULL;
		}

		//     printf("\n\nJob uploaded job id: %u\n\n", cur_job.id);
		  

		//     if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
		//     {
		//         printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
		//         return NULL;;
		//     }


		if ( error = devctl( fd, DCMD_ELCORE_JOB_ENQUEUE, &cur_job.id, sizeof(cur_job.id), NULL ) )
		{
		    printf( "DCMD_ELCORE_JOB_ENQUEUE error: %s\n", strerror ( error ) );
		    return NULL;;
		}

		//     printf("\n\nProg started\n\n");

		int job_status = cur_job.id;

		if ( error = devctl( fd, DCMD_ELCORE_JOB_WAIT, &job_status, sizeof(job_status), NULL ) )
		{
		    printf( "DCMD_ELCORE_JOB_STATUS error: %s\n", strerror ( error ) );
		    return NULL;;
		}

		//     printf("job rc: %d\n", job_status);



		//     if ( error = devctl( fd, DCMD_ELCORE_STOP, NULL, 0, NULL ) )
		//     {
		//         printf( "DCMD_ELCORE_STOP error: %s\n", strerror ( error ) );
		//         return NULL;;
		//     }
		//     
		//     printf("\n\nProg stopped\n\n");

		//     if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
		//     {
		//         printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
		//         return NULL;;
		//     }

		if (error = devctl( fd, DCMD_ELCORE_JOB_RESULTS, &cur_job.id, sizeof(cur_job.id), NULL ) )
		{
		    printf( "DCMD_ELCORE_JOB_RESULTS error: %s\n", strerror ( error ) );
		    return NULL;;
		}
		//     
		//     printf("\n\nProg downloaded\n\n");





		if (error = devctl( fd, DCMD_ELCORE_JOB_RELEASE, &cur_job.id, sizeof(cur_job.id), NULL ) )
		{
		    printf( "DCMD_ELCORE_JOB_RELEASE error: %s\n", strerror ( error ) );
		    return NULL;
		}
    
//     mem_dump(arg_data, DSP_JOBS_COUNT * 2 * sizeof(uint32_t));
    
	}
    return NULL;
}

int main( int argc, char** argv )
{
    uint32_t    job_status;
    uint32_t code_size, arg_size; //size of program
    uint8_t *code_addr;
    uint64_t code_paddr, arg_paddr;
    pthread_t *threads;
    int i;
  
    
	fd = open("/dev/elcore0", O_RDWR);

	if ( fd < 0 )
	{
		perror("error opening file");
		return -1;
	}

	if (argc > 1)
	{
		DSP_JOBS_COUNT = atoi(argv[1]);
	}
	

	if (argc > 2)
	{
		DSP_ITERATIONS = atoi(argv[2]);
	}
    
  
    
	if ((code_addr = mmap(NULL, DSP_PROG_MAX_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("SRC mmap err");
		goto exit0;
	}
	
	arg_size = sizeof(uint32_t) * 2 * DSP_JOBS_COUNT;
	
    if ((arg_data = mmap(NULL, arg_size , PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("SRC mmap err");
		goto exit1;
	}

	if (getbytes(code_addr, fw_path, &code_size) < 0)
    {
		printf("Getbytes error\n");
		goto exit2;
    }

   
	
    if (mem_offset64(code_addr, NOFD, 1, &code_paddr, 0) == -1)
	{
		perror("Get src_phys addr error");
		goto exit2;
	}
    printf("%s: src_phys_1 0x%08x\n", __func__, code_paddr);
   
    if (mem_offset64(arg_data, NOFD, 1, &arg_paddr, 0) == -1)
	{
		perror("Get src_phys addr error");
		goto exit2;
	}
    printf("%s: src_phys_2 0x%08x\n", __func__, code_paddr);
    

    
    for (i = 0; i < DSP_JOBS_COUNT; i++)
    {
        (*(uint32_t*)(arg_data + sizeof(uint32_t) * i * 2)) = 0x11110000 * (i+1);
        (*(uint32_t*)(arg_data + sizeof(uint32_t) * (i * 2 + 1))) = 0x1111 * (i+1);
    }
    
    mem_dump(arg_data, DSP_JOBS_COUNT * 2 * sizeof(uint32_t));
    
    if ((threads = malloc(sizeof(pthread_t) * DSP_JOBS_COUNT)) == NULL)
    {
        goto exit2;
    }
    
    ELCORE_JOB job = {
        .core = 1,
        .inum = 1,
        .onum = 1,
        .code = {code_size, code_paddr}
    };
    

    
    for (i = 0; i < DSP_JOBS_COUNT; i++)
    {
        job.input[0].size = sizeof(uint32_t) * 2;
        job.input[0].client_paddr = arg_paddr + sizeof(uint32_t) * 2 * i;
        job.output[0].size = sizeof(uint32_t);
        job.output[0].client_paddr = arg_paddr + sizeof(uint32_t) * 2 * i;
        
        if (pthread_create(&threads[i], NULL, work_thread, &job))
        {
            goto exit3;
        }
    }

    for (i=0; i < DSP_JOBS_COUNT; i++)
    {
        pthread_join(threads[i], NULL);
    }
    
    mem_dump(arg_data, DSP_JOBS_COUNT * 2 * sizeof(uint32_t));
    
    for (i = 0; i < DSP_JOBS_COUNT; i++)
    {
        printf ("Got for %u thread SUMM: 0x%08x\n", i, (*(uint32_t*)(arg_data + sizeof(uint32_t) * i * 2)));

    }
    
    
    exit3:
        free(threads);
    exit2:
        munmap(arg_data, arg_size);
    exit1:
        munmap(code_addr, DSP_PROG_MAX_SIZE);
    exit0:
        close(fd);
    
    return 0;
    
}
