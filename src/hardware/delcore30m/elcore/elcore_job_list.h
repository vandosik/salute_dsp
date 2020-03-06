
#ifndef		_ELCORE_JOBLIST_H
#define		_ELCORE_JOBLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

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

enum elcore_wait_job {
	ELCORE_WAIT_BLOCK = 0, /*thread blocks at devctl call untill job finishes*/
	ELCORE_WAIT_NONBLOCK /*thread gets immidiate responce about job status*/
};  

//public part of job
typedef struct _elcore_job_entry {
	uint32_t				id;
	enum elcore_job_status	status;
	enum elcore_job_result	rc;
	uint8_t					cores;
// 	unsigned int inum; /* actual number of input arguments */
// 	unsigned int onum; /* actual number of output arguments */
// 
// 	int input[MAX_INPUTS];
// 	int output[MAX_OUTPUTS];
} ELCORE_JOB;


typedef struct _elcore_job {
	ELCORE_JOB				job_pub;
	int						rcvid;
	uint32_t				code_paddr;
	uint32_t				code_len;
	struct _elcore_job		*next;
} elcore_job_t;

//funcs to operate with jobs list
//------------------------------------------
//create job and set id
elcore_job_t* alloc_job(void);

elcore_job_t* get_job_first( void *hdl );

elcore_job_t* get_job_last( void *hdl);

elcore_job_t* get_job_by_id( void *hdl , uint32_t id);

int add_job_last( void *hdl, elcore_job_t* job);

uint32_t jobs_count(void *hdl);

int		remove_job_by_id( void *hdl , uint32_t id);















#endif
