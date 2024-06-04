#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <string.h>
#include <time.h>
#include <sys/wait.h>

#define QUEUE_SIZE 5

char* get_text() {
    srand(getpid());
    int num = rand() % 10 + 1;
    srand(time(NULL) + num+rand());
    char* text = (char*)malloc(11 * sizeof(char));
    if (text == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }
    for (int j = 0; j < 10; j++) {
        text[j] = 'a' + rand() % 26;
    }
    text[10] = '\0';
    return text;
}

void printer(int printer, char text[11]) {
    for (int i = 0; i < 10; i++) {
        printf("Printer %d is printing: %c\n", printer, text[i]);
        fflush(stdout);
        sleep(1);
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <number_of_users> <number_of_printers>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int users = atoi(argv[1]);
    int printers = atoi(argv[2]);

    key_t sem_key = ftok("/tmp", 's');
    int semid = semget(sem_key, 1, IPC_CREAT | 0666);
    if (semid == -1) {
        perror("semget");
        exit(EXIT_FAILURE);
    }

    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } sem_arg;

    struct sembuf semaphore_dec = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
    sem_arg.val = 1;
    if (semctl(semid, 0, SETVAL, sem_arg) == -1) {
        perror("semctl");
        exit(EXIT_FAILURE);
    }

    key_t shm_key = ftok("/tmp", 'm');
    int shmid = shmget(shm_key, QUEUE_SIZE * sizeof(char[11]), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget");
        exit(EXIT_FAILURE);
    }

    char (*queue)[QUEUE_SIZE][11] = shmat(shmid, NULL, 0);
    if (queue == (void *) -1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < QUEUE_SIZE; i++) {
        (*queue)[i][0] = '\0';
    }

    for (int i = 0; i < users; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            int user = i + 1;
            while (1) {
                char* text = get_text();
                semop(semid, &semaphore_dec, 1); 
                int index = -1;
                for (int j = 0; j < QUEUE_SIZE; j++) {
                    if ((*queue)[j][0] == '\0') {
                        strcpy((*queue)[j], text);
                        free(text);
                        break;
                    }
                }
                sem_arg.val = 1; 
                if (semctl(semid, 0, SETVAL, sem_arg) == -1) {
                    perror("semctl");
                    exit(EXIT_FAILURE);
                }
                sleep(rand() % 5 + 1); 
            }
            exit(EXIT_SUCCESS); 
        }
    }

    for (int i = 0; i < printers; i++) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            int print = i + 1;
            while (1) {
                semop(semid, &semaphore_dec, 1); 
                char text[11];
                text[0] = '\0';
                int index = -1;
                for (int j = 0; j < QUEUE_SIZE; j++) {
                    if ((*queue)[j][0] != '\0') {
                        strcpy(text, (*queue)[j]);
                        (*queue)[j][0] = '\0';
                        index = j;
                        break;
                    }
                }
                sem_arg.val = 1; 
                if (semctl(semid, 0, SETVAL, sem_arg) == -1) {
                    perror("semctl");
                    exit(EXIT_FAILURE);
                }
                if (text[0] != '\0') {
                    printer(print, text);
                }
            }
            exit(EXIT_SUCCESS); 
        }
    }

    for (int i = 0; i < users + printers; i++) {
        wait(NULL);
    }

    if (semctl(semid, 0, IPC_RMID) == -1) {
        perror("semctl");
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
    }

    return 0;
}
