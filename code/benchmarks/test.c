#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
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

void alarm(int signum) {
	printf("ur a bitch\n");
}

int main(int argc, char **argv) {

    struct sigaction sa;
	memset (&sa, 0, sizeof(sa));

    // Configure the sigaction struct
    sa.sa_handler = alarm;      // Set the handler function
    sigaction(SIGPROF, &sa, NULL);           // No signals are blocked during execution
    sa.sa_flags = 0;                    // Use default flags (no special behavior)

	struct itimerval timer;

    // Set timer for 2 seconds with 1-second interval
    timer.it_value.tv_sec = 2;          // Initial expiration: 2 seconds
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1;       // Periodic expiration: 1 second
    timer.it_interval.tv_usec = 0;

	
}
