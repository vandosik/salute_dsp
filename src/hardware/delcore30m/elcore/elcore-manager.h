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
// 
// 
    typedef struct {
        uint32_t	mode;
//     #define	SPI_MODE_CHAR_LEN_MASK	(0xFF)		/* Charactor length */
//     #define	SPI_MODE_CKPOL_HIGH		(1 <<  8)
//     #define	SPI_MODE_CKPHASE_HALF	(1 <<  9)
//     #define	SPI_MODE_BODER_MSB		(1 << 10)
//     #define	SPI_MODE_CSPOL_MASK		(1 << 11)	/* Chip select polarity */
//     #define	SPI_MODE_CSPOL_HIGH		(1 << 11)
//     #define	SPI_MODE_CSSTAT_HIGH	(1 << 12)
//     #define	SPI_MODE_CSHOLD_HIGH	(1 << 13)
//     #define	SPI_MODE_RDY_MASK		(3 << 14)	/* Ready signal control */
//     #define	SPI_MODE_RDY_NONE		(0 << 14)
//     #define	SPI_MODE_RDY_EDGE		(1 << 14)	/* Falling edge signal */
//     #define	SPI_MODE_RDY_LEVEL		(2 << 14)	/* Low level signal */
//     #define	SPI_MODE_IDLE_INSERT	(1 << 16)
// 
//     #define	SPI_MODE_LOCKED			(1 << 31)	/* The device is locked by another client */

        uint32_t	clock_rate;
    } elcore_cfg_t;
// 
//     #define	SPI_DEV_ID_MASK			0xFFFF
//     #define	SPI_DEV_ID_NONE			SPI_DEV_ID_MASK
// 
enum elcore_core {
	ELCORE_DEV_CORE_0 = 0x1,
	ELCORE_DEV_CORE_1 = 0x2,
	ELCORE_DEV_CORE_ALL = 0x3,
};

//     /* For SPI API interface */
//     #define	SPI_DEV_DEFAULT			(1 << 31)	/* Default device, use by spi_setcfg()/spi_getdevinfo() call */
//     #define	SPI_DEV_LOCK			(1 << 30)	/* Lock device, for spi_read()/spi_write()/spi_exchange() */
//     #define	SPI_DEV_UNLOCK			(1 << 29)	/* Unlock device, for spi_read()/spi_write()/spi_exchange() */
// 
//     /* For SPI driver interface */
//     #define SPI_DEV_XFER_MASK		3
//     #define SPI_DEV_XFER_SHIFT		16
//     #define SPI_DEV_EXCHANGE		3
//     #define SPI_DEV_READ			1
//     #define SPI_DEV_WRITE			2
// 
    typedef struct {
        uint32_t	device;		/* Device ID */
        char		name[16];	/* Device description */
        elcore_cfg_t	cfg;		/* Device configuration */
    } elcore_devinfo_t;
// 
// 
//     /*
//      * Resource Manager Interface
//      */
//     #define	_IOMGR_SPI	(_IOMGR_PRIVATE_BASE + 0x01)
// 
//     typedef struct {
//         io_msg_t	msg_hdr;
//     #define	_SPI_IOMSG_READ			0x0001
//     #define	_SPI_IOMSG_WRITE		0x0002
//     #define	_SPI_IOMSG_EXCHANGE		0x0003
//     #define	_SPI_IOMSG_CMDREAD		0x0004
//     #define	_SPI_IOMSG_DMAXCHANGE	0x0005
//         uint32_t	device;
//         int32_t		xlen;
//     } spi_msg_t;
// 
//     /*
//      * DMA buffer address : physical address
//      */
//     typedef struct {
//         uint64_t	rpaddr;
//         uint64_t	wpaddr;
//     } spi_dma_paddr_t;
// 
// 
    /*
     * The following devctls are used by a client application
     * to control the SPI interface.
     */
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

#define MAX_INPUTS		1
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

#include <elcore_job_list.h>

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

#define DCMD_ELCORE_DMARECV		__DIOTF (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 3, elcore_dmarecv_t)

#define DCMD_ELCORE_START		__DION (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 4)

#define DCMD_ELCORE_STOP		__DION (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 5)

#define DCMD_ELCORE_PRINT		__DION (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 6)

#define DCMD_ELCORE_RESET		__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 7, int)
//send job id, get status
#define DCMD_ELCORE_JOB_STATUS	__DIOTF (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 8, uint32_t)
//send job id, wait for complete, get result
#define DCMD_ELCORE_JOB_WAIT	__DIOTF (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 9, uint32_t)

#define DCMD_ELCORE_JOB_CREATE	__DIOTF (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 10, ELCORE_JOB)

#define DCMD_ELCORE_JOB_ENQUEUE	__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 11, uint32_t)
//TODO: set results then job is finished
#define DCMD_ELCORE_JOB_RESULTS	__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 12, uint32_t)

#define DCMD_ELCORE_JOB_CANCEL	__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 13, uint32_t)

#define DCMD_ELCORE_JOB_RELEASE	__DIOT (_DCMD_ELCORE, _DCMD_ELCORE_CODE + 14, uint32_t)

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


} elcore_funcs_t;

extern elcore_funcs_t elcore_funcs; //need to pass low lewel funcs to the resmgr

/*
 * Low-level entry, has to be at the beginning of low-level handle
 */
typedef struct _elcore_dev_entry_t {
	iofunc_attr_t				attr;
	uint32_t					cores_num;	//quantity of cores
	void						*hdl;		/* Pointer to high-level handle */
	void						*job_hdl;	/* Set it here to easyly use on both levels*/
} ELCORE_DEV;


#endif


