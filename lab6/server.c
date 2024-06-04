#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>

#define MAX_CLIENTS 15
#define SERVER_KEY 1111

struct message {
    long mtype; 
    int client_id; 
    char mtext[256]; 
};

int main() {
    int server_queue_id;
    int client_queue_id[MAX_CLIENTS];
    int client_count = 0;
    if ((server_queue_id = msgget(SERVER_KEY, IPC_CREAT | 0666)) == -1) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }

    printf("Server is running\n");
    while (1) {
        struct message msg;
        if (msgrcv(server_queue_id, &msg, sizeof(msg.mtext), 0, 0) == -1) {
            perror("msgrcv");
            exit(EXIT_FAILURE);
        }

        if (strncmp(msg.mtext, "INIT", 4) == 0) {
            key_t client_key = ftok("/tmp", msg.client_id);
            client_queue_id[client_count] = msgget(client_key, IPC_CREAT | 0666);
            msg.client_id = client_count++;
            msg.mtype = msg.client_id + 1; 
            sprintf(msg.mtext, "%d", msg.client_id);
            if (msgsnd(client_queue_id[msg.client_id], &msg, sizeof(msg.mtext), 0) == -1) {
                perror("msgsnd");
                exit(EXIT_FAILURE);
            }
        } else {
            for (int i = 0; i < client_count; i++) {
                if (i != msg.client_id) {
                    if (msgsnd(client_queue_id[i], &msg, sizeof(msg.mtext), 0) == -1) {
                        perror("msgsnd");
                        exit(EXIT_FAILURE);
                    }
                }
            }
        }
    }

    return 0;
}
