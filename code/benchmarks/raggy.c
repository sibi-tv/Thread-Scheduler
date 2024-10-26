#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <ucontext.h>
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

void function1(){
    
    for(int i = 0; i < 300000; i++){
        // if(i % 1000 == 0){
        //     printf("%d\n", i);
        // }
        printf("%d\n", i);
    }

}

void function2(){
    for(int i = 0; i < 1000; i++){
        printf("%d. hello world\n", i);
    }
}

int main(int argc, char **argv) {

    pthread_create(&a, NULL, &function1, NULL);
    //pthread_create(&b, NULL, &function2, NULL);

    pthread_join(a, NULL);
    //pthread_join(b, NULL);
    printf("DONE.\n");

    return 0;
}
