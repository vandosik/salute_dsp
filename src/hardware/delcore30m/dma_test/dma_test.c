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
// #include <devctl.h>
#include <errno.h>
// #include <elcore-manager.h>
#include <sdma.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/mman.h>

#define DCMD_CUSTOM	__DIOT (_DCMD_ELCORE, 228 + 5, int)

char* fw_path = "/tmp/input";

// int getbytes(uint8_t** data, const char *filename, uint32_t *size)
// {
// 	printf("%s: entry\n", __func__);
// 
// 	FILE *f = fopen(filename, "r");
// 	if (f == NULL) {
// 		perror("err opening file");
// 		*size = 0;
// 		return -1;
// 	}
// 	fseek(f, 0, SEEK_END);
// 	*size = ftell(f);
// 	fseek(f, 0, SEEK_SET);
// 
// 	printf("size of file: %u\n", *size);
// 	
// 	*data = malloc(*size + sizeof(elcore_send_t));
// 	
// 	if (*data == NULL)
// 	{
// 		perror("err alloc data");
// 	}
// 	
// 	printf ("bytes read: %d\n",fread(*data + sizeof(elcore_send_t), 1, *size, f));
// 
// 	if (*data + sizeof(elcore_send_t) == NULL)
// 	{
// 		perror("err reading data");
// 	}
// 	
// 	fclose(f);
// 
// 	return 0;
// }
// 
// int writefile(uint8_t *data, const char *filename, uint32_t *size)
// {
// 	printf("%s: entry\n", __func__);
// 
// 	FILE *f = fopen(filename, "w");
// 	if (f == NULL) {
// 		perror("err opening file");
// 		*size = 0;
// 		return -1;
// 	}
// 	
// 	
// 	if (data == NULL)
// 	{
// 		perror("No data");
// 	}
// 	
// 	printf ("bytes written: %d\n", fwrite(data, 1, *size, f));
// 
// 	
// 	fclose(f);
// 
// 	return 0;    
// }

#define DMA_TEST_MEM_SIZE       64

int main( int argc, char** argv )
{
    int    job_status;
   
    uint8_t *src_data;
    uint8_t *dst_data;
    
    uint64_t dst_paddr;
    uint64_t src_paddr;

   
    
// #define DST_DYN
    
	if ((src_data = mmap(NULL, DMA_TEST_MEM_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("SRC mmap err");
		goto exit0;
	}
#ifdef DST_DYN 
    if ((dst_data = mmap(NULL, DMA_TEST_MEM_SIZE, PROT_READ | PROT_WRITE | PROT_NOCACHE,
		MAP_PHYS | MAP_ANON, NOFD, 0)) == MAP_FAILED)
	{
		perror("DST mmap err");
		goto exit1;
	}
	
    if (mem_offset64(dst_data, NOFD, 1, &dst_paddr, 0) == -1)
	{
		perror("Get dst_phys addr error");
        goto exit2;
	}
    printf("%s: dst_phys %lld\n", __func__, dst_paddr);
#else 
    
#define DLCR30M_DSP0_PRAM_PHYS				0x3a600000
#define DMA_TEST_DSTADDR                    0xdfba0000

    uint32_t dst_paddr_1;
    printf("input dst addr:\n");
    scanf("%x", &dst_paddr_1);

    printf("got 0x%08x\n", dst_paddr_1);
    dst_paddr = dst_paddr_1;
    
//     dst_paddr = DLCR30M_DSP0_PRAM_PHYS;
    
	if (  (dst_data = mmap_device_memory( NULL, DMA_TEST_MEM_SIZE,
	        PROT_READ | PROT_WRITE | PROT_NOCACHE, 0,
	        dst_paddr )) == MAP_FAILED )
	{
		perror("DST mmap err");
        goto exit1;
	}
#endif
	
    if (mem_offset64(src_data, NOFD, 1, &src_paddr, 0) == -1)
	{
		perror("Get src_phys addr error");
		goto exit2;
	}
    printf("%s: src_phys %lld\n", __func__, src_paddr);




//     set src data
//     strncpy(src_data, "rRaz raz raz this is hardbass, all v sportivkah adidas. And in niki pazani, listen to \
// to hardbass basy!! Op, davai davai, op davai davai. Dominirui and suspend!! \
// abcdefghijklmnopqrstuvwxyz qwertyuiopasdfghjklzxcvbnm",DMA_TEST_MEM_SIZE-1 );
    
    strncpy(src_data, "abcdefghijklmnopqrstuvwxyzABCDEFGHIGKLMNOPQRSTUVWXYZ123456789*:~",DMA_TEST_MEM_SIZE-1 );
    
    src_data[DMA_TEST_MEM_SIZE-1] = '\0';

//     set DEFAULT DST DATA data
    strncpy(dst_data, 
"_______________________________________________________________________________________________________\
_______________________________________________________________________________________________________\
_______________________________________",DMA_TEST_MEM_SIZE-1 );

    
    printf("Default string: \n%s\n", dst_data);
    
    if (sdma_init())
    {
        perror("sdma_init failure");
        goto exit2;
    }
    struct sdma_channel chnl_1 = {
        .rram = NULL,
        .id = 0
    };
    
    struct sdma_exchange sdma_task = {
        .from = (uint32_t)src_paddr,
        .to = (uint32_t)dst_paddr,
        .channel = &chnl_1,
        .size = DMA_TEST_MEM_SIZE,
        .iterations = 1
    };

    if ((job_status = sdma_prepare_task(&sdma_task)) != 0 )
    {
        printf("Job prepare error: %s\n", strerror(-job_status));
        goto exit3;
    }
    
    if ((job_status = sdma_transfer(&sdma_task)) != 0 )
    {
        printf("Job runing error: %s\n", strerror(-job_status));
    }
//     delay(1000);
   
    
    printf("Passed string: \n%s\n", src_data);
    sdma_mem_dump(src_data, DMA_TEST_MEM_SIZE);
    
    dst_data[DMA_TEST_MEM_SIZE-1] = '\0';
    printf("Got string: \n%s\n", dst_data);
    sdma_mem_dump(dst_data, DMA_TEST_MEM_SIZE);
    
    
        sdma_release_task(&sdma_task);
    exit3:
        sdma_fini();
    exit2:
        munmap(dst_data, DMA_TEST_MEM_SIZE);
    exit1:
        munmap(src_data, DMA_TEST_MEM_SIZE);
    exit0:
    
    return 0;
    
}
