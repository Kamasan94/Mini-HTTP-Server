#ifndef __TPOOL_H__
#define __TPOOL_H__

#include<stdbool.h>
#include<stddef.h>
#include<pthread.h>
#include<stdlib.h>

struct tpool;
//Thread pool structure type
typedef struct tpool tpool_t;

typedef void (*thread_func_t)(void* arg);

tpool_t *tpool_create(size_t num);
void tpool_destroy(tpool_t *tm);

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg);
void tpool_wait(tpool_t *tm);

#endif /* __TPOOL_H__ */

//Simple linked list wich stores the function to call and its arguments
struct tpool_work {
	thread_func_t 		func;
	void		  		*arg;
	struct tpool_work 	*next;
};
typedef struct tpool_work tpool_work_t;

//work_first and work_last are used to push and pop work objects
//Single mutex for all locking
struct tpool {
	tpool_work_t	*work_first;
	tpool_work_t	*work_last;
	pthread_mutex_t	work_mutex;	
	pthread_cond_t	work_cond;		//Signals the threads that there is work to be processed 
	pthread_cond_t	working_cond;	//Signals when there are no threads processing
	size_t			working_cnt;	//How many threads are actively processing work
	size_t			thread_cnt;		//Threads alive
	bool			stop;
};

//***************************//
//*******IMPLEMENTATION******//
//**************************//

//Helper to create work object
static tpool_work_t *tpool_work_create(thread_func_t func, void *arg) {
	tpool_work_t *work;

	if(func == NULL) return NULL;

	work = (tpool_work_t *)malloc((sizeof(*work)));
	work -> func = func;
	work -> arg = arg;
	work -> next = NULL;
	return work;
}

//Helper to destroy work object
static void tpool_work_destroy(tpool_work_t *work) {
	if(work == NULL) return;
	free(work);
}

//Function to pull the work from the queue
static tpool_work_t *tpool_work_get(tpool_t *tm) {
	tpool_work_t *work;

	if(tm == NULL) return NULL;

	work = tm->work_first;
	if(work == NULL) return NULL;

	if(work->next == NULL){
		tm->work_first = NULL;
		tm->work_last = NULL;
	}
	else{
		tm->work_first = work->next;
	}

	return work;
}

//Worker function
//Hearth and soul of the pool, where work is handled
static void *tpool_worker(void *arg) {
	tpool_t *tm = (tpool_t*)arg;
	tpool_work_t *work;

	while(1) {
		//First thing locking the mutex, so we are sure nothing else manipulates the pool's members
		pthread_mutex_lock(&(tm->work_mutex));

		//Check if there is any work available for processing and we are still running
		while(tm->work_first == NULL && !tm->stop) 
			pthread_cond_wait(&(tm->work_cond), &(tm->work_mutex));

		//Here we check if the pool has requested that all threads stop running, if so we exit
		if(tm->stop) break;

		//Once the thread was signaled there is work
		work = tpool_work_get(tm);
		tm->working_cnt++;
		pthread_mutex_unlock(&(tm->work_mutex));

		if(work != NULL) {
			work -> func(work->arg);
			tpool_work_destroy(work);
		}

		//Once the work has benn processed
		//1. Mutex locked
		//2. Decremented cnt
		pthread_mutex_lock(&(tm->work_mutex));
		tm->working_cnt--;
		//3.If there is no threads working and no items in the queue a signal will be sent to inform
		//the wai function
		if(!tm->stop && tm->working_cnt == 0 && tm->work_first == NULL)
			pthread_cond_signal(&(tm->working_cond));
		pthread_mutex_unlock(&(tm->work_mutex));
	}


	//We are here beacuse stop is true
	//Decrement the thread count because this thread is stopping
	//We signal tpool_wait that a thread has exited. tpool_wait will wait for all threads to exit when stopping 
	tm->thread_cnt--;
    pthread_cond_signal(&(tm->working_cond));
    pthread_mutex_unlock(&(tm->work_mutex)); //We unlock the mutex here because everything is protected by it
    return NULL;
}


//************************************//
//*******POOL CREATE AND DESTROY******//
//************************************//

tpool_t *tpool_create(size_t num) {
	tpool_t	*tm;
	pthread_t thread;
	size_t i;

	if(num == 0) num = 2;

	tm = (tpool_t*)calloc(1, sizeof(*tm)); //Allocate 1 pool
	tm->thread_cnt = num;

	pthread_mutex_init(&(tm->work_mutex), NULL);
	pthread_cond_init(&(tm->work_cond), NULL);
	pthread_cond_init(&(tm->working_cond), NULL);

	tm->work_first = NULL;
	tm->work_last = NULL;

	for(int i=0; i<num; i++){
		pthread_create(&thread, NULL, tpool_worker, tm);
		pthread_detach(thread);
	}

	return tm;
}


void tpool_destroy(tpool_t *tm) {
	tpool_work_t *work;
	tpool_work_t *work2;

	if(tm == NULL) return;

	pthread_mutex_lock(&(tm->work_mutex));
	work = tm->work_first;
	while(work!=NULL){
		work2 = work->next;
		tpool_work_destroy(work);
		work = work2;
	}
	tm->work_first = NULL;
	tm->stop = true;
	pthread_cond_broadcast(&(tm->work_cond));
	pthread_mutex_unlock(&(tm->work_mutex));

	tpool_wait(tm);

	pthread_mutex_destroy(&(tm->work_mutex));
	pthread_cond_destroy(&(tm->work_cond));
	pthread_cond_destroy(&(tm->working_cond));

	free(tm);
}

//************************************//
//******ADDING WORK TO THE QUEUE******//
//************************************//

bool tpool_add_work(tpool_t *tm, thread_func_t func, void *arg){
	tpool_work_t *work;

	if(tm == NULL) return false;

	work = tpool_work_create(func, arg);

	pthread_mutex_lock(&(tm->work_mutex));
	if(tm->work_first == NULL) {
		tm->work_first = work;
		tm->work_last = tm->work_first;
	}
	else {
		tm->work_last->next = work;
		tm->work_last = work;
	}

	pthread_cond_broadcast(&(tm->work_cond));
	pthread_mutex_unlock(&(tm->work_mutex));

	return true;
}

//Waiting for processing to complete
void tpool_wait(tpool_t *tm) {
	if(tm == NULL) return;

	pthread_mutex_lock(&(tm->work_mutex));
	while(1) {
		if(tm->work_first != NULL 
			|| (!tm->stop && tm->working_cnt != 0) 
			|| (tm->stop && tm->thread_cnt != 0)) 
		{
			pthread_cond_wait(&(tm->working_cond), &(tm->work_mutex));
		}
		else {
			break;
		}
	}
	pthread_mutex_unlock(&(tm->work_mutex));
}