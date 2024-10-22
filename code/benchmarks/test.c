#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "../thread-worker.h"

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

void *printo() {
	// pthread_mutex_lock(&mutex);
	printf("%d\n", x);
	x++;
	// pthread_mutex_unlock(&mutex);
	printf("bruh\n");
}

void *printeee() {
	// pthread_mutex_lock(&mutex);
	printf("%d\n", y);
	y++;
	// pthread_mutex_unlock(&mutex);
	printf("bruh\n");
}


int main(int argc, char **argv) {
	printf("bruh\n");
	
	printf("fuck\n");
	pthread_create(&a, NULL, printo, NULL);
	pthread_create(&b, NULL, printeee, NULL);
	// pthread_join(a, NULL);
	// pthread_join(b, NULL);
	printf("you\n");


}
