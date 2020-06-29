#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

static void *func(void *arg) {
    sleep(3);
    return NULL;
}

int main(void) {
    pthread_t pt;
    int ret = pthread_create(&pt, NULL, func, NULL);
    if (ret == -1) {
        perror("pthread_create");
        return 1;
    }

    char buf[1024];
    ret = pthread_getname_np(pt, buf, 1024);
    if (ret == -1) {
        perror("pthread_getname_np");
        return 1;
    }

    printf("Original thread name=%s\n", buf);

    ret = pthread_setname_np(pt, "TestName");
    if (ret == -1) {
        perror("pthread_setname_np");
        return 1;
    }

    ret = pthread_getname_np(pt, buf, 1024);
    if (ret == -1) {
        perror("pthread_getname_np");
        return 1;
    }

    printf("New thread name=%s\n", buf);

    pthread_join(pt, NULL);
    return 0;
}
