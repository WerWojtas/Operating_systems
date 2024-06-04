#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>

volatile int mode = 0;
int global_num;

void sigusr1_handler(int sig, siginfo_t *sender_info, void *context) {
    kill(sender_info->si_pid, SIGUSR1);
    if (mode != sender_info->si_value.sival_int) {
        global_num++;
    }
    mode = sender_info->si_value.sival_int;

    if (mode == 1) {
        for (int i = 1; i <= 100; ++i) {
            printf("%d\n", i);
        }
    } else if (mode == 2) {
        printf("Liczba żądań zmiany trybu pracy: %d\n", global_num);
    } else if (mode == 3) {
        exit(0);
    }
}

int main() {
    global_num = 0;
    struct sigaction action;
    action.sa_sigaction = sigusr1_handler;
    action.sa_flags = SA_SIGINFO;
    sigaction(SIGUSR1, &action, NULL);

    pid_t pid = getpid();
    printf("PID catcher'a: %d\n", pid);

    while (1) {
        pause();
    }

    return 0;
}
