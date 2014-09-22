#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int k = 0;

void *threadFn(void *arg)
{
    usleep(1);
    int old;
    do {
        old = k;
    } while (__sync_val_compare_and_swap(&k, old, old+1) != old);
    return NULL;
}

int main(int argc, char **argv)
{
    int threads = atoi(argv[1]);
    pthread_t *tids = alloca(sizeof(pthread_t) * threads);
    int i;
    for (i = 0; i < threads; ++i) {
        pthread_create(&tids[i], NULL, threadFn, NULL);
    }
    for (i = 0; i < threads; ++i) {
        pthread_join(tids[i], NULL);
    }
    printf("k == %d\n", k);
    return 0;
}
