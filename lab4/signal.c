#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>


void sigusr1_handler() {
    printf("Otrzymano sygnał SIGUSR1\n");
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Bad number of arguments\n");
        return 1;
    }


    if (strcmp(argv[1], "none") == 0) {
    } else if (strcmp(argv[1], "ignore") == 0) {
        signal(SIGUSR1, SIG_IGN); 
    } else if (strcmp(argv[1], "handler") == 0) {
        signal(SIGUSR1, sigusr1_handler); 
    } else if (strcmp(argv[1], "mask") == 0) {
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGUSR1);
        sigprocmask(SIG_BLOCK, &mask, NULL); 
    } else {
        fprintf(stderr, "Niepoprawny argument: %s\n", argv[1]);
        return 1;
    }

    raise(SIGUSR1);
    sigset_t pending_signals;
    sigpending(&pending_signals);
    if (sigismember(&pending_signals, SIGUSR1)) {
        printf("Sygnał oczekuje na obsłużenie\n");
    } else {
        printf("Brak oczekującego sygnału/ Sygnał został już obsłużony\n");
    }

    return 0;
}
