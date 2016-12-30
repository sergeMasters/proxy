/* queue.h
 * header for jobqueue
 */

#ifndef JOBQUEUE_H_
#define JOBQUEUE_H_
//<------ Includes --------------------------->

#include <pthread.h>
#include <stdlib.h>

//#ifndef PTHREAD_MUTEX_ERRORCHECK
	//#define PTHREAD_MUTEX_ERRORCHECK NULL
//#endif
//<------- Data Structures ------------------------------------->

typedef struct job_queue *JobQueue;// forward declaration of job_queue *JobQueue;
typedef struct job *Job; // foward declaration of job


// struct to represent a job of client request threads
struct job {
	char type; // 'V', 'M', 'P'
	char len; // 1-9
	int clientfd;
	struct job * Next;
};

// struct (Queue) to hold jobs
struct job_queue { 
	pthread_mutex_t m;
	pthread_mutexattr_t attr;
	Job head;
	Job tail;
};

//<------- Functions ------------------------------------->

/* create_jobqueue
 * allocates memory for a new jobqueue and returns it
 * @return JobQueue on success, NULL on failure
 */
JobQueue create_jobqueue();

/* destroy_jobqueue
 * frees jobqueue from memory
 * should be used like this: JQ = destroy_jobqueue(JobQueue JQ);
 * @param JobQueue JQ - JQ to destroy
 * @return NULL
 */
JobQueue destroy_jobqueue(JobQueue JQ);

/* job_create
 * creates a new jon
 * @param
 * @return new Job on success, NULL on failure
 */
Job job_create(int clientfd, char type, char len);

/* destroy_job
 * frees job memory allocation
 * should be used like this: job = destroy_job(job);
 * @param Job job - job to destroy
 * @return NULL
 */
Job destroy_job(Job job);

/* enqueue
 * @param JobQueue JQ - jobqueue to enqueue into
 * @param Job job - job to enqueue
 * @no return
 */
void enqueue(JobQueue JQ, Job job);

/* dequeue
 * @param JobQueue JQ - jobqueue to dequeue a job from 
 * @return a Job if JQ not empty, NULL otherwise
 */
Job dequeue(JobQueue JQ);

#endif /* JOBQUEUE_H_
/* EOF */
