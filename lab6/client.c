#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <string.h>

#define SERVER_KEY 1111

struct message {
    long mtype;
    int client_id;
    char mtext[256]; 
};

int main() {
    int server_queue_id;
    int client_queue_id;
    key_t client_key;
    int client_id;
    pid_t pid;
    if ((client_key = ftok("/tmp", getpid())) == -1) {
        perror("ftok");
        exit(EXIT_FAILURE);
    }
    if ((client_queue_id = msgget(client_key, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    if ((server_queue_id = msgget(SERVER_KEY, 0)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    struct message msg;
    msg.mtype = 1; 
    strcpy(msg.mtext, "INIT");
    msg.client_id = getpid(); 
    if (msgsnd(server_queue_id, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd");
        exit(EXIT_FAILURE);
    }

    if (msgrcv(client_queue_id, &msg, sizeof(msg.mtext), 0, 0) == -1) {
        perror("msgrcv");
        exit(EXIT_FAILURE);
    }
    client_id = atoi(msg.mtext);

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        while (1) {
            if (msgrcv(client_queue_id, &msg, sizeof(msg.mtext), 0, 0) == -1) {
                perror("msgrcv");
                exit(EXIT_FAILURE);
            }
            printf("Client %d received: %s\n", client_id, msg.mtext);
        }
    } else {
        printf("Client %d is running.\n", client_id);
        printf("Type your message: \n");

        while (1) {
            fgets(msg.mtext, sizeof(msg.mtext), stdin);
            msg.mtext[strcspn(msg.mtext, "\n")] = 0; 
            msg.mtype = 1; 
            msg.client_id = client_id;
            if (msgsnd(server_queue_id, &msg, sizeof(msg.mtext), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}
