#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Bad number of arguments\n");
        return 1;
    }

    pid_t catcher_pid = atoi(argv[1]);
    union sigval work_mode;
    work_mode.sival_int = atoi(argv[2]);
    sigqueue(catcher_pid, SIGUSR1, work_mode);
    sigsuspend(NULL);


    return 0;
}
