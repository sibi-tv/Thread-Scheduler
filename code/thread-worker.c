// File:	thread-worker.c

// List all group member's name: Rahulraj Rajesh, Sibi Suriyanarayan Tiruchirapalli Venketaramani
// username of iLab: wax.cs.rutgers.edu
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
tcb *scheduler; // The scheduler's tcb
rq *current_thread; // The current thread we have scheduled onto the queue
rq *found_thread; // Utilized for the findthread method and acts as the pointer for any thread we are trying to find
tcb *benchmark_thread; // The thread of the benchmark code, holding the main thread's context
static ucontext_t *thread_ender; // used to store the context of worker exit and is called when the thread ends
static uint first_call = 1; // used to identify when we first call the worker_create method
static uint thru_yield = 0; // used to identify whether or not the scheduler was swapped to from yield. Because if it was we do not demote a thread's priority.

struct itimerval timer; // The timer that swaps context to another thread
struct itimerval mlfq_timer; // The timer that moves threads from to low to high priority to prevent starvation

// HELPER FUNCTIONS

void ring(){ // Stops the timer and swaps to the scheduler's context (AKA the alarm handler)
	setitimer(ITIMER_PROF, NULL, NULL);
	swapcontext(current_thread->thread->context, scheduler->context);
}

void mlfq_ring() { // Handles the starvation in MLFQ, then swaps context to the scheduler
	setitimer(ITIMER_PROF, NULL, NULL);
	while (runq[LOW_PRIO] != NULL) {
		rq * ptr = dequeue(LOW_PRIO);
		ptr->thread->status = HIGH_PRIO;
		enqueue(ptr);
	}
	swapcontext(current_thread->thread->context, scheduler->context);
}

void enqueue(rq *runny) { // Enqueues the provided thread (runqueue pointer) to the runqueue
	if (runq[runny->thread->priority]) {
		rq *ptr = runq[runny->thread->priority];
		while (ptr->next) {
			ptr = ptr->next;
		}
		ptr->next = runny;
		
	} else {
		runq[runny->thread->priority] = runny;
	}
}


rq* dequeue(uint priority) { // Dequeues the thread (run queue pointer) and updates the runqueue accordingly
	rq *dequeued_rq = runq[priority];
	runq[priority] = runq[priority]->next;
	return dequeued_rq;
}

void find_thread(worker_t th) { // locates the thread based on the provided worker_t id and sets found_thread to this thread
	rq* ptr;
	for (int i = 0; i < NUMPRIO; i++) {
		ptr = runq[i];
		while(ptr != NULL && ptr->thread->id != th) {
			ptr = ptr->next;
		}
	}

	printf("hi%u\n", ptr->thread->id);
	
	found_thread = ptr;
}

void block_thread(rq* blocking_thread, rq* blocked_thread) { // blocks the "blocked_thread" by adding it the "blocking_thread"'s linked list of threads that it blocks
	blocking_thread->thread->blocked_thread=blocked_thread->thread;
}

void mlfq_unlock() { // unlocks any blocked threads in the runqueue that are blocked due to mutexes in MLFQ
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

int psjf_unlock() { // was meant to unblock threads in PSJF
	return EXIT_SUCCESS;
}

static void roundy(int priority){ // round robin implementation

	rq* ptr = dequeue(priority); // dequeues the next thread to run

	while(ptr != NULL && (ptr->thread->status == BLOCKED || ptr->thread->status == EXITED)) { // if the dequeued thread was blocked or it exited, it searches for the next thread

		if (ptr->thread->status == BLOCKED ) {
			ptr->thread->status = READY;
		}

		rq* prev = ptr;
		ptr = dequeue(priority);
		prev->next = NULL;
		enqueue(prev);
	}

	if (ptr == NULL) {
		sched_mlfq(); // call MLFQ again to search the next priority if all the pointers in the queue are blocked
	}


	// schedule the new thread and handle the old thread
	rq *old = current_thread;
	current_thread = ptr;

	current_thread->thread->status = SCHEDULED;
	if (old->thread->status == SCHEDULED) {
		old->thread->status = READY;
	}


	setitimer(ITIMER_PROF, &timer, NULL); // start the timer
	setcontext(current_thread->thread->context); // switch to the new thread's context
	
}


#ifdef MLFQ
/* This function gets called only for MLFQ scheduling set the worker priority. */
int worker_setschedprio(worker_t thread, int prio) {

	find_thread(thread);
	found_thread->thread->priority = prio;	

}
#endif



/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	// - change worker thread's state from Schedulede to Ready
	if (current_thread->thread->status == SCHEDULED) {
		current_thread->thread->status = READY;
	}
	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context
	thru_yield++;
	if(swapcontext(current_thread->thread->context, scheduler->context) < 0){
		perror("set current thread context to scheduler context");
		return EXIT_FAILURE;
	}

	//update # of context switches
	tot_cntx_switches++;
	
	return 0;
};

void worker_exit_wrapper() { // used to set the context for thread_ender
	worker_exit(NULL); 
}

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread
	setitimer(ITIMER_PROF, NULL, NULL); // stops the timer

	current_thread->thread->status = EXITED; // changes the status of the thread to "exited" so that we know it is complete

	// unblocks all threads which were blocked by current thread
	tcb* ptr = current_thread->thread->blocked_thread;
	while(ptr != NULL) {
		ptr->status = READY;
		ptr = ptr->next;
	}

	worker_yield();                                                                                                                                                                                                                                                                        
};

/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
	
	setitimer(ITIMER_PROF, NULL, NULL); // stops the timer


	find_thread(thread); // finds the "blocking thread"
	block_thread(found_thread, current_thread); // adds current thread to the linked list of blokced threads in the "blcoking_thread"'s TCB
	current_thread->thread->status = BLOCKED; // set status of current_thread to blocked

	worker_yield();
	
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

	while (__sync_lock_test_and_set(mutex->lock, 1)) {
		current_thread->thread->status = BLOCKED;
		worker_yield(); // context switch to the scheduler thread
	}
	// - if the mutex is acquired successfully, enter the critical section

	return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	__sync_lock_release(mutex->lock);
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.
#ifndef MLFQ
	psjf_unlock();
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

	swapcontext(scheduler->context, current_thread->thread->context);
	printf("done\n");
	// YOUR CODE HERE
}

/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() { // Executes the MLFQ scheduling algorithm, calls helper function roundy to do round robin on each priority
	
	if (thru_yield) {
		thru_yield--;
		enqueue(current_thread);
	} else {
		current_thread->thread->priority--;
	}

	int queue_to_run = HIGH_PRIO;
	for (int i = HIGH_PRIO; i >= LOW_PRIO; i--) {
		if (runq[i] != NULL) {
			queue_to_run = i;
			i = -1;
		}
	}

	roundy(queue_to_run);
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
	

	// - schedule policy
	#ifndef MLFQ
		printf("round robin\n");
		sched_psjf();
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
	setitimer(ITIMER_PROF, NULL, NULL); // stops the timer
	
	if (first_call) {
		/* initialize benchmark thread */
		benchmark_thread = (tcb*) malloc(sizeof(tcb));
		
		benchmark_thread->context = (ucontext_t*)malloc(sizeof(ucontext_t));

		if (getcontext(benchmark_thread->context) < 0) {
			perror("getcontext");
			free(benchmark_thread->context);
			free(benchmark_thread);
			return EXIT_FAILURE;
		}
		benchmark_thread->priority = HIGH_PRIO;
		benchmark_thread->id = next_thread_id++;

		one_dim_enqueue(benchmark_thread);

		rq *arr_cue = (rq*)malloc(sizeof(rq));
		arr_cue->thread = benchmark_thread;

		
		/*initialize thread_ender */
		current_thread = arr_cue;

		thread_ender =  (ucontext_t*)malloc(sizeof(ucontext_t));
		if (getcontext(thread_ender) < 0) {
			perror("getcontext");
			free(thread_ender);
			return EXIT_FAILURE;
		}

		void *thread_ender_stack = malloc(STACK_SIZE); 

		thread_ender->uc_link = NULL; //current_thread->thread->context;
		thread_ender->uc_stack.ss_sp = thread_ender_stack;
		thread_ender->uc_stack.ss_size = STACK_SIZE;
		thread_ender->uc_stack.ss_flags = 0;

		makecontext(thread_ender, worker_exit_wrapper, 0);
		

		/* initialize the scheduler*/

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

		scheduler->context->uc_link = NULL;
		scheduler->context->uc_stack.ss_sp = schedule_stack;
		scheduler->context->uc_stack.ss_size = STACK_SIZE;
		scheduler->context->uc_stack.ss_flags = 0;

		makecontext(scheduler->context, &schedule, 0);

		//TIMER
		struct sigaction sa;
		memset(&sa, 0, sizeof(sa));
		sa.sa_handler = &ring; //worker_yield is called after 10 mm
		sigaction(SIGPROF, &sa, NULL);

		timer.it_value.tv_usec = QUANTUM;
		timer.it_value.tv_sec = 0;
		timer.it_interval.tv_usec = QUANTUM;
		timer.it_interval.tv_sec = 0;

		//MLFQ TIMER
		struct sigaction sb;
		memset(&sb, 0, sizeof(sb));
		sb.sa_handler = &mlfq_ring;
		sigaction(SIGPROF, &sb, NULL);

		mlfq_timer.it_value.tv_usec = 5 * QUANTUM;
		mlfq_timer.it_value.tv_sec = 0;
		mlfq_timer.it_interval.tv_usec = 5 * QUANTUM;
		mlfq_timer.it_interval.tv_sec = 0;

		first_call--; // decrement so that we know the next time we run worker_create we don't re-initialize our global variables

	}

	// - create Thread Control Block (TCB) malloc it (once workerthread tcb is gonna disappear) 
	//   --> malloc things that are permanent (almost connected TCB with thread)
	tcb *new_thread = (tcb*)malloc(sizeof(tcb));
	*thread = next_thread_id++;
	new_thread->id = *thread;

	new_thread->context = (ucontext_t*)malloc(sizeof(ucontext_t));

	// - create and initialize the context of this worker thread
	if (getcontext(new_thread->context) < 0) {
		perror("getcontext");
		return EXIT_FAILURE;
	}

	// - allocate space of stack for this thread to run
	void *stack = malloc(STACK_SIZE);
	
	// - make it ready for the execution.
	new_thread->status = READY;
	

	// YOUR CODE HERE
	new_thread->context->uc_link = thread_ender; //current_thread->thread->context;
	new_thread->context->uc_stack.ss_sp = stack;
	new_thread->context->uc_stack.ss_size = STACK_SIZE;
	new_thread->context->uc_stack.ss_flags = 0;

	// setting the thread's context to the provided function --> needs arg somehow
	makecontext(new_thread->context, (void (*)())function, 1, arg);

	rq *arr_cue = (rq*)malloc(sizeof(rq));
	arr_cue->thread = new_thread;

	//GO TO SCHEDULE
	enqueue(arr_cue); // after everything is set, push this thread into run queue and

    return EXIT_SUCCESS; // return whether it was successful or not
};

