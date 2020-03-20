#include <elcore_job_list.h>
#include <elcore-manager.h>
#include <errno.h>


void* elcore_job_hdl_init(uint32_t cores_num)
{
	printf("%s: entry\n", __func__);
    
	elcore_job_hdl_t			*job_hdl;
	if ( (job_hdl = calloc(1, sizeof(elcore_job_hdl_t))) == NULL)
	{
		return NULL;
	}
	//TODO: do we need this dynamic allocation?
	job_hdl->cores_num = cores_num;
	job_hdl->core_jobs_max = calloc(cores_num * 2, sizeof(uint32_t));
	job_hdl->core_jobs_cnt = job_hdl->core_jobs_max + cores_num;
	
	if ( !job_hdl->core_jobs_max || !job_hdl->core_jobs_cnt )
	{
		return NULL;
	}
	
	return job_hdl;
}

int set_core_jobs_max( void* hdl, uint32_t core_num, uint32_t val )
{
	printf("%s: entry try set core%u  to %u\n", __func__, core_num, val);
	ELCORE_DEV			*drvhdl = hdl;
    elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl;
	
	
	job_hdl->core_jobs_max[core_num] = val;
	
	return 0;
	
}

//TODO: BUG: need release jobs here?
void elcore_job_hdl_fini(void* hdl)
{
	printf("%s: entry\n", __func__);
	ELCORE_DEV			*drvhdl = hdl;
    elcore_job_hdl_t	*job_hdl = drvhdl->job_hdl;
	int it = 0;
	
	for (; it < job_hdl->cores_num; it++)
	{
		if (job_hdl->core_jobs_cnt[it] > 0)
		{
			printf("%s: not all jobs released\n", __func__);
			
			return;
		}
	}
	
	free(job_hdl->core_jobs_max);
	
	free(job_hdl);
	
	
}

//TODO: make smth more complex
static void gen_job_id(uint32_t* id)
{
	static uint32_t		id_gen = 0; //variable to set unique ids to jobs
	*id = id_gen++;
}

static int job_put_to_storage(void *hdl, elcore_job_t* job )
{
    printf("%s: entry\n", __func__);
	ELCORE_DEV			*drvhdl = hdl;
    elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl;
	elcore_job_t		*tmp_job = job_hdl->storage;
	
	if (!tmp_job)
	{
		job_hdl->storage = job;
		job->next = NULL;
		return 0;
	}
	while (tmp_job->next)
	{
		tmp_job = tmp_job->next;
	}
	
	tmp_job->next = job;
	job->next = NULL;
	job->job_pub.status = ELCORE_JOB_IDLE;

	return 0;
}

elcore_job_t* alloc_job(void *hdl, ELCORE_JOB* job_pub)
{
	printf("%s: entry\n", __func__);
	elcore_job_t*		new_job;
	ELCORE_DEV			*drvhdl = hdl;
	elcore_job_hdl_t	*job_hdl = drvhdl->job_hdl;
	
	if ( !(job_hdl->core_jobs_cnt[job_pub->core] < job_hdl->core_jobs_max[job_pub->core]) )
	{
		printf("No vacant jobs for core %u\n", job_pub->core);
		return NULL;
	}
	
	if ((new_job = calloc(1, sizeof(elcore_job_t))) == NULL)
	{
		perror("Error allocating job");
		return NULL;
	}
	new_job->job_pub = *(job_pub);
	gen_job_id(&new_job->job_pub.id); 
	++(job_hdl->core_jobs_cnt[job_pub->core]);
	
	job_put_to_storage(hdl, new_job);
	
	
	
	return new_job;
}

static int job_remove_from_storage(elcore_job_t* storage, elcore_job_t* trg_job)
{
    printf("%s: entry\n", __func__);
	elcore_job_t	*tmp_job = storage, *job_del;

	
	if (!tmp_job)
	{
		return -1;
	}
	
	if (tmp_job == trg_job)
	{
		storage = tmp_job->next;
		
		return 0;
	}
	while (tmp_job->next)
	{
		if (tmp_job->next == trg_job)
		{
			job_del = tmp_job->next;
			tmp_job->next = job_del->next;
			
			return 0;
		}
		tmp_job = tmp_job->next;
	}
	return -1;
}

int release_job(void *hdl, elcore_job_t* job )
{
	printf("%s: entry\n", __func__);
	ELCORE_DEV			*drvhdl = hdl;
    elcore_job_hdl_t	*job_hdl = drvhdl->job_hdl;
	
	if (job == NULL || job->job_pub.status != ELCORE_JOB_IDLE)
	{
		errno = EINVAL;
		return -1;
	}
	
	if ( job_remove_from_storage(job_hdl->storage, job) )
	{
		errno = EINVAL;
		return -1;
	}
	
	--(job_hdl->core_jobs_cnt[job->job_pub.core]);
	
	free(job);
	
	
	return 0;
	
}

int job_enqueue( void *hdl, elcore_job_t* job )
{
	printf("%s: entry\n", __func__);
	ELCORE_DEV			*drvhdl = hdl;
    elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl;
	elcore_job_t		*tmp_job = job_hdl->queue;
	
	if (job == NULL || job->job_pub.status != ELCORE_JOB_IDLE)
	{
		errno = EINVAL;
		return -1;
	}
	
	if ( job_remove_from_storage(job_hdl->storage, job) )
	{
		errno = EINVAL;
		return -1;
	}
	
	if (!tmp_job)
	{
		job_hdl->queue = job;
		job->next = NULL;
        job->job_pub.status = ELCORE_JOB_ENQUEUED;
		return 0;
	}
	while (tmp_job->next)
	{
		tmp_job = tmp_job->next;
	}
	
	tmp_job->next = job;
	job->next = NULL;
	job->job_pub.status = ELCORE_JOB_ENQUEUED;
    
	return 0;
}

int job_remove_from_queue( void *hdl, elcore_job_t* job )
{
	printf("%s: entry\n", __func__);
	ELCORE_DEV			*drvhdl = hdl;
    elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl;
	elcore_job_t		*tmp_job = job_hdl->queue, *job_del;
	
	if (job == NULL || job->job_pub.status == ELCORE_JOB_IDLE)
	{
		errno = EINVAL;
		return -1;
	}
	
	if (tmp_job == job)
	{
		job_hdl->queue = tmp_job->next;
		tmp_job->job_pub.status = ELCORE_JOB_IDLE;
        
        job_put_to_storage(hdl, tmp_job);
		
		return 0;
	}
	while (tmp_job->next)
	{
		if (tmp_job->next == job)
		{
			job_del = tmp_job->next;
			tmp_job->next = job_del->next;
			job_del->job_pub.status = ELCORE_JOB_IDLE;
			
            job_put_to_storage(hdl, tmp_job);
            
			return 0;
		}
		tmp_job = tmp_job->next;
	}
	return -1;
}

// elcore_job_t* get_job_first( void *hdl )
// {
// 	ELCORE_DEV			*drvhdl = hdl;
//     elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl
// 	
// 	return job_hdl->first_job;
// }

// elcore_job_t* get_job_first_enqueued( void *hdl )
// {
// 	ELCORE_DEV			*drvhdl = hdl;
//     elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl
// 	elcore_job_t		*tmp_job = job_hdl->queue;
// 	
// 	if (!tmp_job)
// 	{
// 		return NULL;
// 	}
// 	do
// 	{
// 		if (tmp_job->job_pub.status == ELCORE_JOB_ENQUEUED)
// 		{
// 			return tmp_job;
// 		}
// 		tmp_job = tmp_job->next;
// 	} while (tmp_job);
// 
// 	return NULL;
// }

//TODO: may more than jne core do job?
elcore_job_t* get_job_first_enqueued( void *hdl, uint8_t core )
{
	printf("%s: entry\n", __func__);
	ELCORE_DEV			*drvhdl = hdl;
    elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl;
	elcore_job_t		*tmp_job = job_hdl->queue;
	
	if (!tmp_job)
	{
		return NULL;
	}
	do
	{
		if (tmp_job->job_pub.status == ELCORE_JOB_ENQUEUED && tmp_job->job_pub.core ==  core)
		{
			return tmp_job;
		}
		tmp_job = tmp_job->next;
	} while (tmp_job);

	return NULL;
}

// elcore_job_t* get_job_last( void *hdl )
// {
// 	ELCORE_DEV		*drvhdl = hdl;
// 	elcore_job_t	*tmp_job = drvhdl->first_job;
// 	
// 	if (!tmp_job)
// 	{
// 		return NULL;
// 	}
// 	while (tmp_job->next)
// 	{
// 		tmp_job = tmp_job->next;
// 	}
// 	return tmp_job;
// }

elcore_job_t* get_enqueued_by_id( void *hdl , uint32_t id)
{
	printf("%s: entry\n", __func__);
	ELCORE_DEV			*drvhdl = hdl;
    elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl;
	elcore_job_t		*tmp_job = job_hdl->queue;
	
	if (!tmp_job)
	{
		return NULL;
	}
	if (tmp_job->job_pub.id == id)
	{
		return tmp_job;
	}
	while (tmp_job->next)
	{
		tmp_job = tmp_job->next;

		if (tmp_job->job_pub.id == id)
		{
			return tmp_job;
		}
	}
	return NULL;
}

elcore_job_t* get_stored_by_id( void *hdl , uint32_t id)
{
	printf("%s: entry\n", __func__);
	ELCORE_DEV			*drvhdl = hdl;
    elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl;
	elcore_job_t		*tmp_job = job_hdl->storage;
	
	if (!tmp_job)
	{
		return NULL;
	}
	if (tmp_job->job_pub.id == id)
	{
		return tmp_job;
	}
	while (tmp_job->next)
	{
		tmp_job = tmp_job->next;

		if (tmp_job->job_pub.id == id)
		{
			return tmp_job;
		}
	}
	return NULL;
}




// int		remove_job_by_id( void *hdl , uint32_t id)
// {
// 	ELCORE_DEV		*drvhdl = hdl;
// 	elcore_job_t	*tmp_job = drvhdl->first_job, *job_del;
// 
// 	
// 	if (!tmp_job)
// 	{
// 		return -1;
// 	}
// 	
// 	if (tmp_job->job_pub.id == id)
// 	{
// 		drvhdl->first_job = tmp_job->next;
// 		free(tmp_job);
// 		
// 		return 0;
// 	}
// 	while (tmp_job->next)
// 	{
// 		if (tmp_job->next->job_pub.id == id)
// 		{
// 			job_del = tmp_job->next;
// 			tmp_job->next = job_del->next;
// 			free(job_del);
// 			
// 			return 0;
// 		}
// 		tmp_job = tmp_job->next;
// 	}
// 	return -1;
// }

























