#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "../tpool.h"
#include <unistd.h>

static const size_t num_test_threads = 4;
static const size_t num_test_items = 100;

void test_worker(void *arg){
    int *val = (int*)arg;
    int old = *val;

    *val += 1000;
    printf("tid=%p, old=%d, val=%d\n", (void *)pthread_self(), old, *val);

    if(*val%2)
        usleep(100000);
}

void test_thread(){
    //Multi-thread configuration
    //Initialize thrad pool
    tpool_t *tm;
    int *vals;
    size_t i;

    tm = tpool_create(num_test_threads);
    vals = (int *)calloc(num_test_items, sizeof(*vals));

    for(i=0; i<num_test_items; i++){
        vals[i] = i;
        tpool_add_work(tm, test_worker, vals+i);
    }

    tpool_wait(tm);

    for(i=0; i<num_test_items; i++){
        printf("%d\n", vals[i]);
    }

    free(vals);
    tpool_destroy(tm);

    return;
}