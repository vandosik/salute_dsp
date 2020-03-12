#include <elcore_job_list.h>
#include <elcore-manager.h>
#include <errno.h>

static uint32_t		id_gen = 0; //variable to set unique ids to jobs

void* elcore_job_hdl_init(void)
{
	printf("%s: entry\n", __func__);
	return calloc(1, sizeof(elcore_job_hdl_t));
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
	elcore_job_t* new_job;
	
	if ((new_job = calloc(1, sizeof(elcore_job_t))) == NULL)
	{
		perror("Error allocating job");
		return NULL;
	}
	new_job->job_pub = *(job_pub);
	new_job->job_pub.id = ++id_gen; //TODO: make smth more complex
	
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
    elcore_job_hdl_t	*job_hdl = (elcore_job_hdl_t*)drvhdl->job_hdl;
	
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
		
		return 0;
	}
	while (tmp_job->next)
	{
		if (tmp_job->next == job)
		{
			job_del = tmp_job->next;
			tmp_job->next = job_del->next;
			job_del->job_pub.status = ELCORE_JOB_IDLE;
			
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

// int add_job_last( void *hdl, elcore_job_t* job )
// {
// 	ELCORE_DEV		*drvhdl = hdl;
// 	elcore_job_t	*tmp_job = drvhdl->first_job;
// 	
// 	if (!tmp_job)
// 	{
// 		drvhdl->first_job = job;
// 		job->next = NULL;
// 		return 0;
// 	}
// 	while (tmp_job->next)
// 	{
// 		tmp_job = tmp_job->next;
// 	}
// 	
// 	tmp_job->next = job;
// 	job->next = NULL;
// 	return 0;
// }


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

























