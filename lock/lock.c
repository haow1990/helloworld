#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

volatile int value = 0;
int count;

#ifdef SPIN
pthread_spinlock_t spin;
#endif

#ifdef MUTEX
pthread_mutex_t mutex;
#endif

#ifdef NOLOCK
void *threadFn(void *param)
{
    int i = count;
    while (i > 0) {
        ++value;
        --i;
    }
    return NULL;
}
#endif

#ifdef CAS
void *threadFn(void *param)
{
    int i = count;
    int old;
    while (i > 0) {
        do {
            old = value;
        } while(__sync_val_compare_and_swap(&value, old, old + 1) != old);
        --i;
    }
    return NULL;
}
#endif

#ifdef SPIN
void *threadFn(void *param)
{
    int i = count;
    int old;
    while (i > 0) {
        pthread_spin_lock(&spin);
        ++value;
        pthread_spin_unlock(&spin);
        --i;
    }
    return NULL;
}
#endif

#ifdef ATOMIC
void *threadFn(void *param)
{
    int i = count;
    int old;
    while (i > 0) {
        __sync_fetch_and_add(&value, 1);
        --i;
    }
    return NULL;
}
#endif

#ifdef MUTEX
void *threadFn(void *param)
{
    int i = count;
    int old;
    while (i > 0) {
        pthread_mutex_lock(&mutex);
        ++value;
        pthread_mutex_unlock(&mutex);
        --i;
    }
    return NULL;
}
#endif

int main(int argc, char **argv)
{
    int threads = atoi(argv[1]);
    count = atoi(argv[2]);
    pthread_t *tids = alloca(sizeof(pthread_t) * threads);
#ifdef SPIN
    pthread_spin_init(&spin, 0);
#endif
#ifdef MUTEX
    pthread_mutex_init(&mutex, NULL);
#endif
    int i;
    for (i = 0; i < threads; ++i) {
        pthread_create(tids+i, NULL, threadFn, NULL);
    }
    for (i = 0; i < threads; ++i) {
        pthread_join(tids[i], NULL);
    }
    printf("value = %d\n", value);
    return 0;
}

