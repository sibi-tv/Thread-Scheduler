// File:	thread-worker.c

// List all group member's name: Rahulraj Rajesh, Sibi Suriyanarayan Tiruchirapalli Venketaramani
// username of iLab:
// iLab Server:

#include "thread-worker.h"

//Global counter for total context switches and 
//average turn around and response time
long tot_cntx_switches=0;
double avg_turn_time=0;
double avg_resp_time=0;


// INITAILIZE ALL YOUR OTHER VARIABLES HERE
rq *runq[NUMPRIO]; // might have to be a double pointer
rq *last;
tcb scheduler; // this somehow relates to the benchmark stuff
static uint first_call = 1;

/* create a new thread */
int worker_create(worker_t * thread, pthread_attr_t * attr, void *(*function)(void*), void * arg) {

	/* How do you use worker_t */
	if (first_call) {
		if (getcontext(&scheduler) < 0) {
			perror("getcontext");
			return EXIT_FAILURE;
		}

		void *schedule_stack = malloc(STACK_SIZE);

		// YOUR CODE HERE
		scheduler.context->uc_link = NULL;
		scheduler.context->uc_stack.ss_sp = schedule_stack;
		scheduler.context->uc_stack.ss_size = STACK_SIZE;
		scheduler.context->uc_stack.ss_flags = 0;

		makecontext(&scheduler, schedule, 0);
	}

	// - create Thread Control Block (TCB) malloc it (once workerthread tcb is gonna disappear) --> malloc things that are permanent (almost connected TCB with thread)
	tcb *thready = malloc(sizeof(tcb));
	*thread = next_thread_id++;
	(*thready).id = thread;

	// - create and initialize the context of this worker thread
	if (getcontext(&((*thready).context)) < 0) {
		perror("getcontext");
		return EXIT_FAILURE;
	}

	// - allocate space of stack for this thread to run
	void *stack = malloc(STACK_SIZE);

	// after everything is set, push this thread into run queue and
	

	#ifndef MLFQ
	// Use PSJF priority queue
	#else
		rq new_thread;
		new_thread.thread = &thready; 
		ENQUEUE(&new_thread, );
	#endif
	
	// - make it ready for the execution.
	(*thready).status = READY;

	// YOUR CODE HERE
	(*thready).context->uc_link = NULL;
	(*thready).context->uc_stack.ss_sp = stack;
	(*thready).context->uc_stack.ss_size = STACK_SIZE;
	(*thready).context->uc_stack.ss_flags = 0;

	// setting the thread's context to the provided function --> needs arg somehow
	makecontext((*thready).context, function, 1, arg);

	/*
		Needs timer stuff
		Arugment stuff
	*/


    return EXIT_SUCCESS; // return whether it was successful or not
};


#ifdef MLFQ
/* This function gets called only for MLFQ scheduling set the worker priority. */
int worker_setschedprio(worker_t thread, int prio) {


   // Set the priority value to your thread's TCB
   // YOUR CODE HERE

   return 0;	

}
#endif



/* give CPU possession to other user-level worker threads voluntarily */
int worker_yield() {
	
	// - change worker thread's state from Running to Ready

	// - save context of this thread to its thread control block
	// - switch from thread context to scheduler context

	// YOUR CODE HERE
	
	return 0;
};

/* terminate a thread */
void worker_exit(void *value_ptr) {
	// - de-allocate any dynamic memory created when starting this thread

	// YOUR CODE HERE
};


/* Wait for thread termination */
int worker_join(worker_t thread, void **value_ptr) {
	
	// - wait for a specific thread to terminate
	// - de-allocate any dynamic memory created by the joining thread
  
	// YOUR CODE HERE
	return 0;
};

/* initialize the mutex lock */
int worker_mutex_init(worker_mutex_t *mutex, 
                          const pthread_mutexattr_t *mutexattr) {
	//- initialize data structures for this mutex

	// YOUR CODE HERE
	return 0;
};

/* aquire the mutex lock */
int worker_mutex_lock(worker_mutex_t *mutex) {

        // - use the built-in test-and-set atomic function to test the mutex
        // - if the mutex is acquired successfully, enter the critical section
        // - if acquiring mutex fails, push current thread into block list and
        // context switch to the scheduler thread

        // YOUR CODE HERE
        return 0;
};

/* release the mutex lock */
int worker_mutex_unlock(worker_mutex_t *mutex) {
	// - release mutex and make it available again. 
	// - put threads in block list to run queue 
	// so that they could compete for mutex later.

	// YOUR CODE HERE
	return 0;
};


/* destroy the mutex */
int worker_mutex_destroy(worker_mutex_t *mutex) {
	// - de-allocate dynamic memory created in worker_mutex_init

	return 0;
};

/* scheduler */
static void schedule() {
	// - every time a timer interrupt occurs, your worker thread library 
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
	// Choose PSJF
#else 
	// Choose MLFQ
#endif

}

/* actual implementation of schedulers go here*/

/* Pre-emptive Shortest Job First (POLICY_PSJF) scheduling algorithm */
static void sched_psjf() {
	// - your own implementation of PSJF
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}


/* Preemptive MLFQ scheduling algorithm */
static void sched_mlfq() {
	// - your own implementation of MLFQ
	// (feel free to modify arguments and return types)

	// YOUR CODE HERE
}

//DO NOT MODIFY THIS FUNCTION
/* Function to print global statistics. Do not modify this function.*/
void print_app_stats(void) {

       fprintf(stderr, "Total context switches %ld \n", tot_cntx_switches);
       fprintf(stderr, "Average turnaround time %lf \n", avg_turn_time);
       fprintf(stderr, "Average response time  %lf \n", avg_resp_time);
}


// Feel free to add any other functions you need

void enqueue(rq *new_thread, uint priority) {
	if (runq == NULL) {
		runq[priority] = &new_thread;
		last = runq;
	} else {
		last->next = &new_thread;
		last = last->next;
	}
}

rq* dequeue(uint priority) {
	rq *dequeued_thread = runq[priority];
	runq[priority] = runq[priority]->next;
	return dequeued_thread;
}

