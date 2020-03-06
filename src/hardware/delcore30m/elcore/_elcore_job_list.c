#include <elcore_job_list.h>
#include <elcore-manager.h>
#include <errno.h>

static uint32_t		id_gen = 0; //variable to set unique ids to jobs


elcore_job_t* alloc_job(void)
{
	elcore_job_t* new_job;
	
	if ((new_job = calloc(1, sizeof(elcore_job_t))) == NULL)
	{
		perror("Error allocating job");
		return NULL;
	}
	new_job->job_pub.id = ++id_gen; //TODO: make smth more complex
	
	return new_job;
}

elcore_job_t* get_job_first( void *hdl )
{
	ELCORE_DEV		*drvhdl = hdl;
	
	return drvhdl->first_job;
}

elcore_job_t* get_job_first_enqueued( void *hdl )
{
	ELCORE_DEV		*drvhdl = hdl;
	elcore_job_t	*tmp_job = drvhdl->first_job;
	
	if (!tmp_job)
	{
		return NULL;
	}
	do
	{
		if (tmp_job->job_pub.status == ELCORE_JOB_ENQUEUED)
		{
			return tmp_job;
		}
		tmp_job = tmp_job->next;
	} while (tmp_job);

	return NULL;
}

elcore_job_t* get_job_last( void *hdl )
{
	ELCORE_DEV		*drvhdl = hdl;
	elcore_job_t	*tmp_job = drvhdl->first_job;
	
	if (!tmp_job)
	{
		return NULL;
	}
	while (tmp_job->next)
	{
		tmp_job = tmp_job->next;
	}
	return tmp_job;
}

elcore_job_t* get_job_by_id( void *hdl , uint32_t id)
{
	ELCORE_DEV		*drvhdl = hdl;
	elcore_job_t	*tmp_job = drvhdl->first_job;
	
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

int add_job_last( void *hdl, elcore_job_t* job )
{
	ELCORE_DEV		*drvhdl = hdl;
	elcore_job_t	*tmp_job = drvhdl->first_job;
	
	if (!tmp_job)
	{
		drvhdl->first_job = job;
		job->next = NULL;
		return 0;
	}
	while (tmp_job->next)
	{
		tmp_job = tmp_job->next;
	}
	
	tmp_job->next = job;
	job->next = NULL;
	return 0;
}

uint32_t jobs_count(void *hdl)
{
	ELCORE_DEV		*drvhdl = hdl;
	elcore_job_t	*tmp_job = drvhdl->first_job;
	uint32_t		count = 0;
	
	if (!tmp_job)
	{
		return 0;
	}
	do
	{
		tmp_job = tmp_job->next;
		count++;
	} while (tmp_job);

	return count;
}

int		remove_job_by_id( void *hdl , uint32_t id)
{
	ELCORE_DEV		*drvhdl = hdl;
	elcore_job_t	*tmp_job = drvhdl->first_job, *job_del;

	
	if (!tmp_job)
	{
		return -1;
	}
	
	if (tmp_job->job_pub.id == id)
	{
		drvhdl->first_job = tmp_job->next;
		free(tmp_job);
		
		return 0;
	}
	while (tmp_job->next)
	{
		if (tmp_job->next == id)
		{
			job_del = tmp_job->next;
			tmp_job->next = job_del->next;
			free(job_del);
			
			return 0;
		}
		tmp_job = tmp_job->next;
	}
	return -1;
}

























