#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <stdio.h>

static void handler(int signum, siginfo_t *info, void *secret) {
    printf("Parent PID=%d\n", (int)(info->si_pid));
    exit(0);
}

int main() {
    pid_t pid = getpid();
    printf("PID = %d\n", (int)pid);

    pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    } else if (pid == 0) {
        // child
        struct sigaction action;
        action.sa_sigaction = handler;
        sigemptyset(&action.sa_mask);
        action.sa_flags = SA_SIGINFO;

        int ret = prctl(PR_SET_PDEATHSIG, SIGUSR1);
        if (ret < 0) {
            perror("prctl(PR_SET_PDEATHSIG)");
            return 1;
        }

        sigaction(SIGUSR1, &action, NULL);
        pause();
    } else {
        // parent
        sleep(2);
        printf("parent process exits\n");
    }
    return 0;
}
