
#ifndef		_ELCORE_JOBLIST_H
#define		_ELCORE_JOBLIST_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <elcore-manager.h>


typedef struct _elcore_job {
	ELCORE_JOB				job_pub;
	int						rcvid;
	uint8_t					mem_part;		//part of dsp core's memory, allocated by job
	uint32_t				code_dspaddr;		//addr of code for job in dsp addrs
	uint32_t				code_cpuaddr;		//addr of code for job in cpu addrs
	uint32_t				input_dspaddr[MAX_INPUTS]; //in dsp addressing
	uint32_t				output_dspaddr[MAX_OUTPUTS]; //in dsp adressing
	uint32_t				input_cpupaddr[MAX_INPUTS]; //in cpu adressing
	uint32_t				output_cpupaddr[MAX_INPUTS]; //in cpu adressing
	uint32_t				sdma_chaincount;
	struct sdma_exchange	sdma_chains[DSP_MAX_CHAINS];
    
	struct _elcore_job		*next;
} elcore_job_t;

typedef struct _elcore_job_hdl {
	elcore_job_t			*storage; // ELCORE_JOB_IDLE jobs
	elcore_job_t			*queue; // ELCORE_JOB_ENQUEUED and ELCORE_JOB_RUNNING jobs
	int						cores_num;
	uint32_t				*core_jobs_max; //max jobs fore core
	uint32_t				*core_jobs_cnt; //current quantity of jobs for core
} elcore_job_hdl_t;

//funcs to operate with jobs list

void* elcore_job_hdl_init(uint32_t cores_num);

void elcore_job_hdl_fini(void *hdl);
//------------------------------------------
//create job and set id
elcore_job_t* alloc_job(void *hdl, ELCORE_JOB* job_pub);

int release_job(void *hdl, elcore_job_t* job );

int set_core_jobs_max( void* hdl, uint32_t core_num, uint32_t val );

int job_enqueue( void *hdl, elcore_job_t* job );

int job_remove_from_queue( void *hdl, elcore_job_t* job );

elcore_job_t* get_job_first_enqueued( void *hdl, uint8_t core );

elcore_job_t* get_enqueued_by_id( void *hdl , uint32_t id);

elcore_job_t* get_stored_by_id( void *hdl , uint32_t id);
//TODO:
int release_all( void*hdl );

#endif
