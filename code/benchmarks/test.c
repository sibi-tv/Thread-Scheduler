#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <ucontext.h>
#include <pthread.h>
//#include "../thread-worker.h"

/* A scratch program template on which to call and
 * test thread-worker library functions as you implement
 * them.
 *
 * You can modify and use this program as much as possible.
 * This will not be graded.
 */
//pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_t a, b;
int x = 0;
int y = 1;

void *foo() {
	// pthread_mutex_lock(&mutex);
	while(1) printf("foo\n");
}
void *bar() {
	// pthread_mutex_lock(&mutex);
	printf("bar\n");
}

struct itimerval timer;
ucontext_t cctx, nctx;


void alarm_handler(int signum) {
	printf("Switch to bar\n");
    //printf("%ld\n", timer.it_interval.tv_sec);
    swapcontext(&cctx, &nctx);
}



int main(int argc, char **argv) {

    struct sigaction sa;
	memset (&sa, 0, sizeof(sa));
    sa.sa_handler = &alarm_handler;      // Set the handler function

    // Set timer for 2 seconds with 1-second interval
    timer.it_value.tv_sec = 0;          // Initial expiration: 2 seconds
    timer.it_value.tv_usec = 1;
    timer.it_interval.tv_sec = 1;       // Periodic expiration: 1 second
    timer.it_interval.tv_usec = 0;

    setitimer(ITIMER_PROF, &timer, NULL);

    getcontext(&cctx);
    cctx.uc_stack.ss_sp = malloc(8192);
    cctx.uc_stack.ss_size = 8192;
    makecontext(&cctx, (void (*)())foo, 0);

    getcontext(&nctx);
    nctx.uc_stack.ss_sp = malloc(8192);
    nctx.uc_stack.ss_size = 8192;
    makecontext(&nctx, (void (*)())bar, 0);

    sigaction(SIGPROF, &sa, NULL);           // No signals are blocked during execution
    //sa.sa_flags = 0;                    // Use default flags (no special behavior)

    setcontext(&cctx);

    return 0;

	
}
