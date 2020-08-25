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


#define DSP_SYNC_STOP_BIT			(1 << 31)
#define DSP_SYNC_CPU_READY			(1 << 30) //cpu set new array
#define DSP_SYNC_DSP_READY			(1 << 29) //dsp counted summ
#define DSP_SYNC_DMA_READY			(1 << 28)
#define DSP_DMA_CHANNEL     2

char* fw_path_1 = "/tmp/arr_sum_iter";

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
	//printf("%s: entry\n", __func__);

	FILE *f = fopen(filename, "r");
	if (f == NULL) {
		perror("err opening file");
		*size = 0;
		return -1;
	}
	fseek(f, 0, SEEK_END);
	*size = ftell(f);
	fseek(f, 0, SEEK_SET);

	//printf("size of file: %u\n", *size);
	
	printf ("%s: bytes read: %d\n",__func__, fread(data, 1, *size, f));

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
	//printf("%s: entry\n", __func__);

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
	
	//printf ("bytes written: %d\n", fwrite(data, 1, *size, f));

	
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
    uint8_t *src_data_1, *arg_data ;
    uint64_t src_paddr_1, arg_paddr;
  
    
    fd = open("/dev/elcore0", O_RDWR);
    
    if ( fd < 0 )
    {
        perror("error opening file");
        return -1;
    }
    
    
    
    
	if ((src_data_1 = mmap(NULL, DMA_TEST_MEM_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("SRC mmap err");
		goto exit0;
	}
	
    uint32_t arr_len;
   
    printf("Input array len: \n >");
    if ( scanf("%u", &arr_len) < 0)
    {
        goto exit2;
    }
	
	size_2 = (arr_len + 3) * sizeof(uint32_t);
	
   
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
   
    printf("\nProg has been read from file\n");
	
    if (mem_offset64(src_data_1, NOFD, 1, &src_paddr_1, 0) == -1)
	{
		perror("Get src_phys addr error");
		goto exit2;
	}
    //printf("%s: src_phys_1 0x%08x\n", __func__, src_paddr_1);
   
    if (mem_offset64(arg_data, NOFD, 1, &arg_paddr, 0) == -1)
	{
		perror("Get src_phys addr error");
		goto exit2;
	}
    
//     int i;
//     printf("Input array by space \n >");
//     
//     for (i = 0; i < arr_len; i++)
//     {
//         if ( scanf("%u", ((uint32_t*)(arg_data + 4 + i * 4))) < 0)
//         {
//             goto exit2;
//         }
//         
//     }

    uint32_t *array_len = (uint32_t*)arg_data;
    uint32_t *array = (uint32_t*)(arg_data + sizeof(uint32_t));
    uint32_t arr_paddr = arg_paddr + sizeof(uint32_t);
    uint32_t *sync_addr = (uint32_t*)(arg_data + arr_len * sizeof(uint32_t) + sizeof(uint32_t));
    uint32_t sync_paddr = arg_paddr + arr_len * sizeof(uint32_t) + sizeof(uint32_t);
    uint32_t *dsp_summ = (uint32_t*)(arg_data + arr_len * sizeof(uint32_t) + 2 * sizeof(uint32_t));
   
    uint32_t cpu_summ = 0;
    
    *array_len = arr_len; //количество чисел
    *dsp_summ = 0;
    *sync_addr = sync_paddr; //передаем в качестве параметра физический адрес регистра синхъронизации
   


   
    int i;
    
    for ( i = 0; i < arr_len; i++)
    {
        array[i] = rand() % 1000;
        printf("  %x  ", array[i]);
        cpu_summ += array[i]; 
    }

    printf("\n");
    
    ELCORE_JOB arr_sum_job = {
        .core = 0,
        .inum = 3,
        .input[0] = {sizeof(uint32_t), arg_paddr},
        .input[1] = {arr_len * sizeof(uint32_t), arr_paddr},
        .input[2] = {sizeof(uint32_t), sync_paddr},
        .onum = 1,
        .output[0] = {sizeof(uint32_t), arg_paddr + (arr_len * sizeof(uint32_t) + 2 * sizeof(uint32_t))},
        .code = {size_1, src_paddr_1}
    };
    

    

    
    printf(" in0 %08x,\n in1 %08x \n, in2 %08x\n, out0 %08x\n",
           arg_paddr, arg_paddr + sizeof(uint32_t), sync_paddr, arg_paddr + (arr_len * sizeof(uint32_t) + 2 * 
sizeof(uint32_t)));
    
    
    
    
    printf("count: %u \t result: %u\n", (*array_len), (*dsp_summ));
    
    if (error = devctl( fd, DCMD_ELCORE_JOB_CREATE, &arr_sum_job, sizeof(ELCORE_JOB), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_CREATE error: %s\n", strerror ( error ) );
        goto exit2;
    }
    
    printf("\nJob uploaded job id: %u\n", arr_sum_job.id);
    
    uint8_t* sdma_data;
	uint32_t sdma_len = sizeof(SDMA_CHAIN) + sizeof(struct sdma_descriptor) * 1;
	
    if ((sdma_data = malloc(sdma_len)) == NULL)
    {
        perror("mmap sdma fail");
        goto exit2;        
    }
    
    SDMA_CHAIN	*sdma_task = (SDMA_CHAIN*)sdma_data;
    
       sdma_task->channel = DSP_DMA_CHANNEL;
       sdma_task->chain_size = 1; //package number
       sdma_task->job_id = arr_sum_job.id;
       sdma_task->from = arr_paddr;
       sdma_task->to = DSP_SDMA_INPUT(0);
       

    
    struct sdma_descriptor *sdma_package = (struct sdma_descriptor*)(sdma_data + sizeof(SDMA_CHAIN));
    
	sdma_package->f_off = 0; //offset from sdma_exchange->from
	sdma_package->t_off = 0;
	sdma_package->iter = 100; // количество повторов отправки данного пакета от 1 до 255, 0 - повторять бесконечно
	sdma_package->size = arr_len * sizeof(uint32_t);

    
    if ( error = devctl( fd, DCMD_ELCORE_SET_SDMACHAIN, sdma_data, sdma_len, NULL ) )
    {
        printf( "DCMD_ELCORE_SET_SDMACHAIN error: %s\n", strerror ( error ) );
        goto exit3;
    }
    
    
    
    if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
        goto exit3;
    }
    
    
    if ( error = devctl( fd, DCMD_ELCORE_JOB_ENQUEUE, &arr_sum_job.id, sizeof(arr_sum_job.id), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_ENQUEUE error: %s\n", strerror ( error ) );
        goto exit3;
    }
 
    printf("\nProg started\n");
    
	int it;
	
	//repeat summing 10 
	for (it = 0; it < 10;it++)
	{
        printf("Wait dsp\n");
        
        devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL );

        
		//wait DSP to count summ
	    while (!(*sync_addr & DSP_SYNC_DSP_READY));
		
		if (error = devctl( fd, DCMD_ELCORE_JOB_RESULTS, &arr_sum_job.id, sizeof(arr_sum_job.id), NULL ) )
		{
			printf( "DCMD_ELCORE_JOB_RESULTS error: %s\n", strerror ( error ) );
			goto exit3;
		}
		
		printf("\tcount: %u \n \tdsp_summ: %u\n \tcpu_summ: %u \n \tsync_reg: %x \n", 
		(*array_len), (*dsp_summ), cpu_summ,
		*sync_addr );
		
		cpu_summ = 0;
			
	    for ( i = 0; i < arr_len; i++)
	    {
	        array[i] = rand() % 1000;
	        printf("  %x  ", array[i]);
	        cpu_summ += array[i]; 
	    }

	    printf("\n");
	    
		*sync_addr |= DSP_SYNC_CPU_READY;
		
		//wait DMA

		printf("Wait dma\n");
        
        devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL );
        printf("\tcount: %u \n \tdsp_summ: %u\n \tcpu_summ: %u \n \tsync_reg: %x \n", 
		(*array_len), (*dsp_summ), cpu_summ,
		*sync_addr );
        
        
        
		//wait transfer to end
	    while (!(*sync_addr & DSP_SYNC_DMA_READY));
        
        devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL );
        printf("\tcount: %u \n \tdsp_summ: %u\n \tcpu_summ: %u \n \tsync_reg: %x \n", 
		(*array_len), (*dsp_summ), cpu_summ,
		*sync_addr );
        
		*sync_addr &= ~DSP_SYNC_CPU_READY;
   
	}

    //stop job
    *sync_addr |= DSP_SYNC_STOP_BIT;
    
    
    job_status = arr_sum_job.id;
   
    if ( error = devctl( fd, DCMD_ELCORE_JOB_STATUS, &job_status, sizeof(job_status), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_STATUS error: %s\n", strerror ( error ) );
        goto exit3;
    }
    
    
    printf("job status: %s\n", job_status == 0? "idle" : "running");
    
    
    printf("\nWaiting for job\n");
    
    job_status = arr_sum_job.id;
    
    if ( error = devctl( fd, DCMD_ELCORE_JOB_WAIT, &job_status, sizeof(job_status), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_WAIT error: %s\n", strerror ( error ) );
        goto exit3;    printf("count: %u \t dsp_summ: %u\n cpu_summ: %u \n sync_reg: %x \n", 
           (*array_len), (*dsp_summ), cpu_summ,
           *sync_addr );
    }

    printf("job result: %s\n", job_status == 0? "success" : "error");
    
    
    
//     if ( error = devctl( fd, DCMD_ELCORE_STOP, NULL, 0, NULL ) )
//     {
//         printf( "DCMD_ELCORE_STOP error: %s\n", strerror ( error ) );
//         goto exit3;
//     }

    printf("\nProg stopped\n");
    
    if ( error = devctl( fd, DCMD_ELCORE_PRINT, NULL, 0, NULL ) )
    {
        printf( "DCMD_ELCORE_PRINT error: %s\n", strerror ( error ) );
        goto exit3;
    }
   
    if (error = devctl( fd, DCMD_ELCORE_JOB_RESULTS, &arr_sum_job.id, sizeof(arr_sum_job.id), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_RESULTS error: %s\n", strerror ( error ) );
        goto exit3;
    }
    
    printf("\nProg downloaded\n");
    
    
    //writefile(src_data_1, "/tmp/output1", &size_1);
//     writefile(arg_data, "/tmp/output2", &size_2);

	
		printf("count: %u \t dsp_summ: %u\n cpu_summ: %u \n sync_reg: %x \n", 
		(*array_len), (*dsp_summ), cpu_summ,
		*sync_addr );
   

    
    if (error = devctl( fd, DCMD_ELCORE_JOB_RELEASE, &arr_sum_job.id, sizeof(arr_sum_job.id), NULL ) )
    {
        printf( "DCMD_ELCORE_JOB_RELEASE error: %s\n", strerror ( error ) );
        goto exit3;
    }
   
    
    exit3:
        free(sdma_data);
    exit2:
        munmap(arg_data, size_2);
    exit1:
        munmap(src_data_1, size_1);
    exit0:
        close(fd);
    
    return 0;
    
}
