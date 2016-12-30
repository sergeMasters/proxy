/* 
 * Job queue implementation
 * queue.c
 * queue for holding jobs
 */
 
//<------ Includes --------------------------->

#include "queue.h"


//<------ functions --------------------------->

/* create_jobqueue
 * allocates memory for a new jobqueue and returns it
 * @return JobQueue on success, NULL on failure
 */
JobQueue create_jobqueue() {
	JobQueue JQ = malloc(sizeof(*JQ));
	if (!JQ) // check memory allocation 
		return NULL;
	JQ->head = JQ->tail = NULL; // initialize fields
	// initialize mutex lock <---
	pthread_mutexattr_init(&(JQ->attr));
	pthread_mutexattr_settype(&(JQ->attr), PTHREAD_MUTEX_ERRORCHECK);
	pthread_mutex_init(&(JQ->m), &(JQ->attr));
	return JQ;
}


/* destroy_jobqueue
 * frees jobqueue from memory
 * should be used like this: JQ = destroy_jobqueue(JobQueue JQ);
 * @param JobQueue JQ - JQ to destroy
 * @return NULL
 */
JobQueue destroy_jobqueue(JobQueue JQ) {
	Job job;
	while (JQ->head) {
		job = JQ->head;
		JQ->head = job->Next;
		job = destroy_job(job);
	}
	// free mutex lock <----
	pthread_mutexattr_destroy(&(JQ->attr));
	pthread_mutex_destroy(&(JQ->m));
	free(JQ);
	return NULL;
}

/* job_create
 * creates a new sub-matrix multiplication job for the JQ
 * @param 
 * @return new Job on success, NULL on failure
 */
Job job_create(int cfd, char type, char len) {
	Job job = malloc(sizeof(*job));
	if (!job)
		return NULL; // memory allocation failure
	job->clientfd = cfd;
	job->type = type;
	job->len = len;

	job->Next = NULL;
	return job;
}


/* destroy_job
 * frees job memory allocation
 * should be used like this: job = destroy_job(job);
 * @param Job job - job to destroy
 * @return NULL
 */
Job destroy_job(Job job){
	free(job);
	return NULL;
}


/* enqueue
 * @param JobQueue JQ - jobqueue to enqueue into
 * @param Job job - job to enqueue
 * @no return
 */
void enqueue(JobQueue JQ, Job job){
	// Lock here <--------
	pthread_mutex_lock(&(JQ->m));
	if(JQ->tail == NULL){
		JQ->head = JQ->tail = job;
	} else {
		JQ->tail->Next = job;
		JQ->tail = job;
	}
	pthread_mutex_unlock(&(JQ->m));
	// Unlock here <-------
	return;
}


/* dequeue
 * @param JobQueue JQ - jobqueue to dequeue a job from 
 * @return a Job if JQ not empty, NULL otherwise
 */
Job dequeue(JobQueue JQ) {
	// Lock here <--------
	pthread_mutex_lock(&(JQ->m));
	if(JQ->head == NULL) {
		pthread_mutex_unlock(&(JQ->m));
		return NULL;
	}
	
	
	Job job = JQ->head;
	JQ->head = JQ->head->Next;
	if (JQ->head == NULL)
		JQ->tail = NULL;
	
	pthread_mutex_unlock(&(JQ->m));
	// Unlock here <-------
	return job;
}
/* EOF */
