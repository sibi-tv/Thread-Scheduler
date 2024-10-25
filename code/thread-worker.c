// File:	thread-worker.c

// List all group member's name: Rahulraj Rajesh, Sibi Suriyanarayan Tiruchirapalli Venketaramani
// username of iLab:
// iLab Server:

#include "thread-worker.h"
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;

// INITAILIZE ALL YOUR OTHER VARIABLES HERE

rq *runq[NUMPRIO]; // The runqueue for MLFQ
rq *thread_list; // A list of threads created for simple searching
rq *thread_list_last; // Holds the ptr of the final node in thread_list
tcb *scheduler; // this somehow relates to the benchmark stuff
rq *current_thread; // yes
tcb *benchmark_thread;
static uint first_call = 1;
static uint mlfq_first_call = 1;
//static int benchmark_unenqueued = 1;

struct itimerval timer;

// HELPER FUNCTIONS

// void alarm(int signum) {
// 	printf("fuck off prick\n");
// } 

void ring(){
	printf("um");
	swapcontext(current_thread->thread->context, scheduler->context);
}

void enqueue(rq *runny) {
	if (runq[runny->thread->priority]) {
		rq *ptr = runq[runny->thread->priority];
		while (ptr->next) {
			ptr = ptr->next;
		}
		ptr->next = runny;
		
	} else {
		runq[runny->thread->priority] = runny;
		// if(benchmark_unenqueued) { // when the very first thread is made, you enqueue the benchmark thread on the same level
        //     rq *new_rq_node = (rq*) malloc(sizeof(rq));
        //     runq[thread_bread_leftonread->priority]->next = new_rq_node;
		// 	new_rq_node->thread = benchmark_thread;
        //     benchmark_unenqueued--;
        // }
	}
}

void one_dim_enqueue(tcb* thread) {
	rq *new_rq_node = (rq*)malloc(sizeof(rq));
	if(thread_list == NULL) {
		thread_list = new_rq_node;
	} else {
		thread_list_last->next = new_rq_node;
	}
	thread_list_last = new_rq_node;
}

rq* dequeue(uint priority) {
	rq *dequeued_rq = runq[priority];
	runq[priority] = runq[priority]->next;
	// tcb *dequeued_thread = dequeued_rq->thread;
	// free(dequeued_rq);
	return dequeued_rq;
}

rq* peek(uint priority) {

}

void mlfq_unlock() {
	uint desired_mutex_lock_id = current_thread->thread->mutex_lock->id;
	for (int i = 0; i < NUMPRIO; i++) {
		rq *ptr = runq[i];
		while(ptr) {
			if (ptr->thread->status == BLOCKED && desired_mutex_lock_id == ptr->thread->mutex_lock->id) { 
				ptr->thread->status = READY;
			}
			ptr = ptr->next;
		}
	}
}

void psjf_unlock() {
	// rag
}


#ifdef MLFQ
/* This function gets called only for MLFQ scheduling set the worker priority. */
int worker_setschedprio(worker_t thread, int prio) {

	rq *ptr = thread_list;
	while (ptr) {
		if (ptr->thread->id == thread) {
			ptr->thread->priority = prio;
		}
	}

   return 0;	

}
#endif



/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready
	current_thread->thread->status = READY;

	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context
	if(swapcontext(current_thread->thread->context, scheduler->context) < 0){
		perror("set current thread context to scheduler context");
		return EXIT_FAILURE;
	}
	// YOUR CODE HERE

	//update # of context switches
	tot_cntx_switches++;
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread
	
	uint tid = current_thread->thread->id;

	// Acquiring and rewiring the thread_list linked list to exclude the current thread being exited
	rq *ptr = thread_list, *ptr2 = NULL;
	if(ptr->thread->id == tid) {
		thread_list = thread_list->next;
		ptr->next = NULL;
	} else {
		while (ptr->next) {
			if(ptr->next->thread->id == tid) {
				ptr2 = ptr->next;
				ptr->next = ptr->next->next;
				ptr2->next = NULL;
				ptr = ptr2;
			} else {
				ptr = ptr->next;
			}
		}
	}

	// deallocation for PSJF?

	// deallocation for MLFQ?
	
	// de-allocating dynamically allocated things
	free(current_thread->thread->context->uc_stack.ss_sp);
	//needs to free more things
	free(current_thread);
	free(ptr);
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread

	/* value.ptr is the return value of thread --> so just get the return value and save it */
	/* *value_ptr = return_value */

	/* how do we know a thread is finished? */

	/*
	In this method, do we completely disregard the scheduler and wait for the entire thread to finish?
	Cuz otherwise why do we deallocate memory HERE?
	--> Nope apprently and I'm stupid confused
	*/

	//needs to call worker exit
	
	//Note: Until schedulers are created, I will leave this blank.
	
	setitimer(ITIMER_PROF, NULL, NULL);	

	current_thread->thread->status = BLOCKED;
	ring();
	
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex
	mutex = (worker_mutex_t*)malloc(sizeof(worker_mutex_t));

	// how do you get the thread info

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

		current_thread->thread->mutex_lock = mutex;

        // - use the built-in test-and-set atomic function to test the mutex
		// sync whatever
		while (__sync_lock_test_and_set(mutex->lock, 1)) {
			// yield
			// use yield --> spin lock uses entire quantum. yield gives quantum back to scheduelr
			current_thread->thread->status = BLOCKED;
			worker_yield();
			/* Don't need this cuz worker_yield takes care of this
			setcontext(scheduler->context); // might have to be a swap context instead
			*/
		}
        // - if the mutex is acquired successfully, enter the critical section
		/* This is just automatically handled */
        // - if acquiring mutex fails, push current thread into block list and
		
        // context switch to the scheduler thread
		

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	__sync_lock_release(mutex->lock);
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.
#ifndef MLFQ
	// Choose PSJF
#else 
	mlfq_unlock();
#endif

	// YOUR CODE HERE
	return 0;
};

/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init
	free(mutex);

	return 0;
};

/* actual implementation of schedulers go here*/



/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	//enqueue();

	//enqueue();
	swapcontext(scheduler->context, current_thread->thread->context);
	printf("done\n");
	// YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)
	


	// YOUR CODE HERE
}

static void roundy(){
	//enqueue();
	// In any scheduler, you swap from the currently running thread to the thread you dequeued
	
	printf("cock mcgee\n");
	rq* ptr = dequeue(HIGH_PRIO);
	printf("%d\n", ptr->thread->id);
	printf("%d\n", ptr->thread->status);

	current_thread->thread->status = READY;

	// if (mlfq_first_call) {
	// 	current_thread = ptr;
	// 	ptr = dequeue(HIGH_PRIO);
	// 	mlfq_first_call--;
	// } else {
	enqueue(current_thread);
	//printf("%d\n", current_thread->thread->id);
	//assuming at least one thread is not BLOCKED
	while(ptr->thread->status == BLOCKED) {
		rq* prev = ptr;
		ptr = dequeue(HIGH_PRIO);
		enqueue(prev);
	}

	rq *old = current_thread;
	current_thread = ptr;

	current_thread->thread->status = SCHEDULED;

	// current_thread = ptr;
	// current_thread->thread->status = SCHEDULED;
	// }

	printf("ok...\n");
	setitimer(ITIMER_PROF, &timer, NULL);
	printf("%d\n", current_thread->thread->context->uc_stack.ss_size);
	printf("%d\n", swapcontext(old->thread->context, current_thread->thread->context));
	
}

/* scheduler */
static void schedule() {
	// - every time a timer interrupt occurs, your worker thread library mutex
	// should be contexted switched from a thread context to this 
	// schedule() function

	// - invoke scheduling algorithms according to the policy (PSJF or MLFQ)

	// if (sched == PSJF)
	//		sched_psjf();
	// else if (sched == MLFQ)
	// 		sched_mlfq();

	// YOUR CODE HERE
	setitimer(ITIMER_PROF, NULL, NULL);

	

	// - schedule policy
	#ifndef MLFQ
		// printf("psjf reached\n");
		// sched_psjf();
		printf("round robin\n");
		roundy();
	#else 
		printf("mlfq reached\n");
		sched_mlfq();
	#endif

}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {
	fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
	fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
	fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {

	//To be safe, stopping the timer
	setitimer(ITIMER_PROF, NULL, NULL);
	
	if (first_call) {

		void *benchmark_stack = malloc(STACK_SIZE);

		/* benchmark thread */
		benchmark_thread = (tcb*) malloc(sizeof(tcb));
		benchmark_thread->context = (ucontext_t*)malloc(sizeof(ucontext_t));
		benchmark_thread->priority = HIGH_PRIO;
		benchmark_thread->id = next_thread_id++;

		benchmark_thread->context->uc_stack.ss_sp = benchmark_stack;
		benchmark_thread->context->uc_stack.ss_size = STACK_SIZE;


		if (getcontext(benchmark_thread->context) < 0) {
			perror("getcontext");
			free(benchmark_thread->context);
			free(benchmark_thread);
			return EXIT_FAILURE;
		}

		

		rq *arr_cue = (rq*)malloc(sizeof(rq));
		arr_cue->thread = benchmark_thread;

		//GO TO SCHEDULE
		// enqueue(arr_cue);

		current_thread = arr_cue;
		
		scheduler = (tcb*)malloc(sizeof(tcb));

		scheduler->context = (ucontext_t*)malloc(sizeof(ucontext_t));

		scheduler->id = next_thread_id++;

		if (getcontext(scheduler->context) < 0) {
			perror("getcontext");
			free(scheduler->context);
			free(scheduler);
			return EXIT_FAILURE;
		}

		void *schedule_stack = malloc(STACK_SIZE);

		// YOUR CODE HERE
		scheduler->context->uc_link = NULL;
		scheduler->context->uc_stack.ss_sp = schedule_stack;
		scheduler->context->uc_stack.ss_size = STACK_SIZE;
		scheduler->context->uc_stack.ss_flags = 0;

		makecontext(scheduler->context, &schedule, 0);

		first_call--;

		//TIMER
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = &ring; //worker_yield is called after 10 mm
		sigaction(SIGPROF, &sa, NULL);

		timer.it_value.tv_usec = 50000;
		timer.it_value.tv_sec = 0;
		timer.it_interval.tv_usec = 0;
		timer.it_interval.tv_sec = 0;
	}

	// - create Thread Control Block (TCB) malloc it (once workerthread tcb is gonna disappear) 
	//   --> malloc things that are permanent (almost connected TCB with thread)
	tcb *new_thread = (tcb*)malloc(sizeof(tcb));
	*thread = next_thread_id++;
	new_thread->id = *thread;

	new_thread->context = (ucontext_t*)malloc(sizeof(ucontext_t));

	new_thread->priority = HIGH_PRIO;

	// - create and initialize the context of this worker thread
	if (getcontext(new_thread->context) < 0) {
		perror("getcontext");
		return EXIT_FAILURE;
	}

	// - allocate space of stack for this thread to run
	void *stack = malloc(STACK_SIZE);

	// after everything is set, push this thread into run queue and
	
	// - make it ready for the execution.
	new_thread->status = READY;

	// YOUR CODE HERE
	new_thread->context->uc_link = current_thread->thread->context;
	new_thread->context->uc_stack.ss_sp = stack;
	new_thread->context->uc_stack.ss_size = STACK_SIZE;
	new_thread->context->uc_stack.ss_flags = 0;

	// setting the thread's context to the provided function --> needs arg somehow
	makecontext(new_thread->context, (void (*)())function, 1, arg);

	one_dim_enqueue(new_thread);

	rq *arr_cue = (rq*)malloc(sizeof(rq));
	arr_cue->thread = new_thread;

	//GO TO SCHEDULE
	enqueue(arr_cue);


	//schedule();

	//setitimer(ITIMER_PROF, &timer, NULL);
	ring();


    return EXIT_SUCCESS; // return whether it was successful or not
};

