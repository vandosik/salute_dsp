/*
 * $QNXLicenseC:
 * Copyright 2007, QNX Software Systems. All Rights Reserved.
 * 
 * You must obtain a written license from and pay applicable license fees to QNX 
 * Software Systems before you may reproduce, modify or distribute this software, 
 * or any work that includes all or part of this software.   Free development 
 * licenses are available for evaluation and non-commercial purposes.  For more 
 * information visit http://licensing.qnx.com or email licensing@qnx.com.
 *  
 * This file may contain contributions from others.  Please review this entire 
 * file for other proprietary rights or license notices, as well as the QNX 
 * Development Suite License Guide at http://licensing.qnx.com/license-guide/ 
 * for other information.
 * $
 */


#ifndef _ELCORE_MASTER_LIB_H_INCLUDED
#define _ELCORE_MASTER_LIB_H_INCLUDED

#include <sys/iofunc.h>
#include <sys/dispatch.h>

#ifndef	__TYPES_H_INCLUDED
#include <sys/types.h>
#endif

#ifndef	_INTTYPES_H_INCLUDED
#include <inttypes.h>
#endif

//     #include <sys/iomsg.h>
// 
//     #define	SPI_VERSION_MAJOR	1
//     #define	SPI_VERMAJOR_SHIFT	16
//     #define	SPI_VERSION_MINOR	0
//     #define	SPI_VERMINOR_SHIFT	8
//     #define SPI_REVISION		0
//     #define	SPI_VERREV_SHIFT	0
// 
// 
    /*
     * DSP driver information
     */
    typedef struct {
        uint32_t	version;
        char		name[16];	/* Driver name */
        uint32_t	feature;
    #define	SPI_FEATURE_DMA			(1 << 31)
    #define	SPI_FEATURE_DMA_ALIGN	0xFF		/* DMA buffer alignment mask, alignment = 2^alignment */
    } elcore_drvinfo_t;
//TODO: temporary core0 - 0, core1 - 1
enum elcore_core {
	ELCORE_DEV_CORE_0 = 0x1,
	ELCORE_DEV_CORE_1 = 0x2,
	ELCORE_DEV_CORE_ALL = 0x3,
};

enum delcore30m_memory_type {
	DELCORE30M_MEMORY_XYRAM,
	DELCORE30M_MEMORY_SYSTEM,
};    

struct elcore_buffer {
// 	enum delcore30m_memory_type			type;
	size_t								size;
    uint32_t							client_paddr;
};

#include <devctl.h>
#define _DCMD_ELCORE				_DCMD_MISC
#define _DCMD_ELCORE_CODE			0x11

enum elcore_job_status {
	ELCORE_JOB_IDLE = 0,
	ELCORE_JOB_ENQUEUED,
	ELCORE_JOB_RUNNING,
};

enum elcore_job_result {
	ELCORE_JOB_ERROR = -2,
	ELCORE_JOB_CANCELLED = -1,
	ELCORE_JOB_SUCCESS = 0,
};

#define MAX_INPUTS		4
#define MAX_OUTPUTS		1

//public part of job
typedef struct _elcore_job_entry {
	uint32_t				id;
	enum elcore_job_status	status;
	enum elcore_job_result	rc;
	uint8_t					core; //TODO: cores was here. One job for one core?
	uint32_t inum; /* actual number of input arguments */
	uint32_t onum; /* actual number of output arguments */
// 
	struct elcore_buffer input[MAX_INPUTS];
	struct elcore_buffer output[MAX_OUTPUTS];
	struct elcore_buffer code;

} ELCORE_JOB;


//TODO: move this somewhere else
#include <sdma.h>

//TODO: make cmth dynamic
#define DSP_MAX_CHAINS				MAX_INPUTS + MAX_OUTPUTS
/*
*Set this macro with number of input arg (begining with 0)to field @to of SDMA_CHAIN struct 
* to write data to input part of DSP memory. This addr is reserved in DSP namespace.
*/
#define DSP_SDMA_INPUT(input_num)		(0x38A00000 + input_num)
/*
*Set this macro with number of output arg (begining with 0) to field @from of SDMA_CHAIN struct 
* to write data to input part of DSP memory This addr is reserved in DSP namespace.
*/
#define DSP_SDMA_OUTPUT(output_num)		(0x3A880000 + output_num)
//call before enqueue

//TODO: move this somewhere else
#include <elcore_job_list.h>

#if 0

typedef struct {
	uint32_t	len;
	uint32_t	offset;
	uint32_t	core;
} elcore_send_t;

#define DCMD_ELCORE_SEND		__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 0, elcore_send_t)

typedef struct {
	uint32_t	len;
	uint32_t	offset;
	uint32_t	core;
} elcore_recv_t;

#define DCMD_ELCORE_RECV		__DIOTF (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 1, elcore_recv_t)

typedef struct {
	uint32_t	len;
	uint32_t	offset;
	uint32_t	core;
    uint32_t	dma_src;
} elcore_dmasend_t;

#define DCMD_ELCORE_DMASEND		__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 2, elcore_dmasend_t)

typedef struct {
	uint32_t	len;
	uint32_t	offset;
	uint32_t	core;
	uint32_t	dma_dst;
} elcore_dmarecv_t;



#define DCMD_ELCORE_DMARECV			__DIOTF (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 3, elcore_dmarecv_t)

#define DCMD_ELCORE_START			__DION (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 4)

#define DCMD_ELCORE_STOP			__DION (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 5)

#define DCMD_ELCORE_RESET			__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 7, int)

#endif

#define DCMD_ELCORE_PRINT			__DION (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 6)
//send job id, get status
#define DCMD_ELCORE_JOB_STATUS		__DIOTF (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 8, uint32_t)
//send job id, wait for complete, get result
//TODO: make timeout
#define DCMD_ELCORE_JOB_WAIT		__DIOTF (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 9, uint32_t)

#define DCMD_ELCORE_JOB_CREATE		__DIOTF (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 10, ELCORE_JOB)

#define DCMD_ELCORE_JOB_ENQUEUE		__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 11, uint32_t)
//TODO: set results then job is finished
#define DCMD_ELCORE_JOB_RESULTS		__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 12, uint32_t)

#define DCMD_ELCORE_JOB_CANCEL		__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 13, uint32_t)

#define DCMD_ELCORE_JOB_RELEASE		__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 14, uint32_t)


/*
*use this struct
*typedef struct _elcore_sdma_chain {
*		uint32_t					job_id;
*		uint32_t					from;
*		uint32_t					to;
*		uint32_t					channel; //channel number
*		uint32_t					chain_size; //in sdma_desc
*		struct sdma_descriptor		*sdma_chain; //цепочка пакетов обмена
*} SDMA_CHAIN;
*/

#define DCMD_ELCORE_SET_SDMACHAIN	__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 15, SDMA_CHAIN)

#define DSP_SDMA_IN_MASK				(~DSP_SDMA_INPUT(0))
#define DSP_SDMA_OUT_MASK				(~DSP_SDMA_OUTPUT(0))
// 
// 
//     /*
//      * SPI API calls
//      */
//     int	spi_open(const char *path);
//     int spi_close(int fd);
//     int spi_setcfg(int fd, uint32_t device, spi_cfg_t *cfg);
//     int spi_getdevinfo(int fd, uint32_t device, spi_devinfo_t *devinfo);
//     int spi_getdrvinfo(int fd, spi_drvinfo_t *drvinfo);
//     int spi_read(int fd, uint32_t device, void *buf, int len);
//     int spi_write(int fd, uint32_t device, void *buf, int len);
//     int spi_xchange(int fd, uint32_t device, void *wbuf, void *rbuf, int len);
//     int spi_cmdread(int fd, uint32_t device, void *cbuf, int16_t clen, void *rbuf, int rlen);
//     int spi_dma_xchange(int fd, uint32_t device, void *wbuf, void *rbuf, int len);
// 
// 


// enum delcore30m_memory_type {
// 	DELCORE30M_MEMORY_XYRAM,
// 	DELCORE30M_MEMORY_SYSTEM,
// };

// struct delcore30m_buffer {
// 	int fd;
// 	enum delcore30m_memory_type type;
// 	int core_num;
// 	size_t size;
// };



typedef struct {
    /* size of this structure */
    size_t	size;

    void*	(*init)(void* hdl, char *options);
    
    void	(*fini)(void *hdl);
    
    uint32_t		(*write)(void *hdl, uint32_t core_num, void* from, void* offset, 
int *size);
    
    uint32_t		(*read)(void *hdl, uint32_t core_num, void* to, void* offset, 
int *size);
	
	int		(*start_core)(void *hdl, uint32_t core_num);
	
	int		(*stop_core)(void *hdl, uint32_t core_num);
   //TODO: may be try to reset using stop/start
	int		(*reset_core)(void *hdl, uint32_t core_num); 
	
	int		(*print)(void *hdl);
    
	int		(*ctl)(void *hdl, int cmd, void *msg, int msglen, int *nbytes, int *info);
   
	int		(*irq_thread)(void *hdl);
    //need this?
	uint32_t		(*dma_send)( void *hdl, uint32_t core_num, uint32_t from, uint32_t offset, int *size);
    //need this?
	uint32_t		(*dma_recv)(void *hdl, uint32_t core_num, uint32_t to,  uint32_t offset, int *size);
    
	int			(*set_prog)( void *hdl, void *job);

	int			(*set_data)( void *hdl, void *job);
    
	int			(*get_data)( void *hdl, void *job);
	
	int			(*release_mem)(void *hdl, void *job);

	int			(*setup_dmachain)(void *hdl, void *chain);
} elcore_funcs_t;

extern elcore_funcs_t elcore_funcs; //need to pass low lewel funcs to the resmgr

/*
 * Low-level entry, has to be at the beginning of low-level handle
 */
typedef struct _elcore_dev_entry_t {
	iofunc_attr_t				attr;
	uint32_t					cores_num;	//quantity of cores, need this?
	void						*hdl;		/* Pointer to high-level handle */
	void						*job_hdl;	/* Set it here to easyly use on both levels*/
} ELCORE_DEV;


#endif


