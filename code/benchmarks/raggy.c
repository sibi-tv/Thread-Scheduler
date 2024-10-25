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
    printf("hi\n");
    for(int i = 0; i < 300000; i++){
        printf("%d\n", i);
    }
}

void function2(){
    for(int i = 0; i < 1000; i++){
        printf("hello world\n");
    }
}

int main(int argc, char **argv) {

    pthread_create(&a, NULL, &function1, NULL);
    //pthread_create(&b, NULL, &function2, NULL);
    //pthread_create(&b, NULL, &function2, NULL);

    printf("Sib n rag\n");

    pthread_join(a, NULL);

    printf("One thread handled\n");

    

    printf("plz");
    
    //pthread_join(b, NULL);

    printf("Done. YEAAAA\n");

    return 0;
}
