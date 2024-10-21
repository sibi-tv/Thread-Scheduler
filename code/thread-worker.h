// File:	worker_t.h

// List all group member's name: Rahulraj Rajesh (rr1185), Sibi Suriyanarayan Tiruchirapalli Venketaramani (st1005)
// username of iLab:
// iLab Server:

#ifndef WORKER_T_H
#define WORKER_T_H

#define _GNU_SOURCE

/* To use Linux pthread Library in Benchmark, you have to comment the USE_WORKERS macro */
#define USE_WORKERS 1

/* include lib header files that you need here: */
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <stdbool.h>

#define STACK_SIZE (50 * 1024)

typedef uint worker_t;

static worker_t next_thread_id = 0;
static worker_t next_thread_id = 0;

typedef struct TCB {
	/* add important states in a thread control block */
	// thread Id
	worker_t *id;

	// thread status
	uint status;

	// thread context
	ucontext_t *context;

	// thread stack --> taken care of by the context???

	// thread priority
	uint priority;

	// And more ...

	worker_mutex_t *mutex_lock;
	/*
	
	Needs time variables
	
	*/


	// YOUR CODE HERE
} tcb; 

/* mutex struct definition */
typedef struct worker_mutex_t {
	uint *id;
	uint *lock;
} worker_mutex_t;

typedef struct hashmap {
	uint key;
	rq *blocked_threads;
} hashmap;

/* Thread States */
#define READY 0
#define SCHEDULED 1
#define BLOCKED 2

/* Priority definitions */
#define NUMPRIO 4
#define HIGH_PRIO 3
#define MEDIUM_PRIO 2
#define DEFAULT_PRIO 1
#define LOW_PRIO 0

/* define your data structures here: */
// Feel free to add your own auxiliary data structures (linked list or queue etc...)

typedef struct runqueue {
	tcb *thread;
	struct runqueue *next;
} rq;

#define ENQUEUE(new_thread) enqueue(new_thread)
#define DEQUEUE() dequeue()


/* Function Declarations: */

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void*(*function)(void*), void * arg); // Sibo AND Rag

/* give CPU pocession to other user level worker threads voluntarily */
int worker_yield(); // Rag

/* terminate a thread */
void worker_exit(void *value_ptr); // Rag

/* wait for thread termination */
int worker_join(worker_t thread, void **value_ptr); // Rag

/* initial the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t *mutexattr); // Sibi

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex); // Sibi

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex); // Sibi

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex); // Sibi


/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void);

#ifdef USE_WORKERS
#define pthread_t worker_t
#define pthread_mutex_t worker_mutex_t
#define pthread_create worker_create
#define pthread_exit worker_exit
#define pthread_join worker_join
#define pthread_mutex_init worker_mutex_init
#define pthread_mutex_lock worker_mutex_lock
#define pthread_mutex_unlock worker_mutex_unlock
#define pthread_mutex_destroy worker_mutex_destroy
#define pthread_setschedprio worker_setschedprio
#endif

#endif
