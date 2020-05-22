#define _GNU_SOURCE
#include <sys/types.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>

#ifndef __NR_pidfd_open
#define __NR_pidfd_open 434 /* System call # on most architectures */
#endif

static int pidfd_open(pid_t pid, unsigned int flags) {
    return syscall(__NR_pidfd_open, pid, flags);
}

int main(int argc, char *argv[]) {
    struct pollfd pollfd;
    int pidfd, ready;

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <pid>\n", argv[0]);
        exit(1);
    }

    pidfd = pidfd_open(atoi(argv[1]), 0);
    if (pidfd == -1) {
        perror("pidfd_open");
        exit(1);
    }

    int old = fcntl(pidfd, F_GETFD);
    if (fcntl(pidfd, F_SETFD, old & ~FD_CLOEXEC) == -1) {
        perror("fcntl(F_SETFD)");
        exit(1);
    }

    pollfd.fd = pidfd;
    pollfd.events = POLLIN;

    ready = poll(&pollfd, 1, -1);
    if (ready == -1) {
        perror("poll");
        exit(EXIT_FAILURE);
    }

    system(argv[2]);
    return 0;
}