
#ifndef		_ELCORE_JOBLIST_H
#define		_ELCORE_JOBLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <elcore-manager.h>



typedef struct _elcore_job {
	ELCORE_JOB				job_pub;
	int						rcvid;
	uint32_t				code_dspaddr;
	uint32_t				input_dspaddr[MAX_INPUTS];
	uint32_t				outpu_dspaddr[MAX_OUTPUTS];
    
	struct _elcore_job		*next;
} elcore_job_t;

typedef struct _elcore_job_hdl {
	uint32_t				storage_cnt; //quantity of jobs created and not placed at the queue yet
	elcore_job_t			*storage;
	uint32_t				queue_cnt; //quantity of jobs in the queue
	elcore_job_t			*queue;
} elcore_job_hdl_t;

//funcs to operate with jobs list

void* elcore_job_hdl_init(void);
//------------------------------------------
//create job and set id
elcore_job_t* alloc_job(void *hdl, ELCORE_JOB* job_pub);

int release_job(void *hdl, elcore_job_t* job );

int job_enqueue( void *hdl, elcore_job_t* job );

int job_remove_from_queue( void *hdl, elcore_job_t* job );

elcore_job_t* get_job_first_enqueued( void *hdl, uint8_t core );

elcore_job_t* get_enqueued_by_id( void *hdl , uint32_t id);

elcore_job_t* get_stored_by_id( void *hdl , uint32_t id);

#endif
