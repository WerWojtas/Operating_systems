#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024
#define MAX_WORDS 10

void error(const char *msg) {
    perror(msg);
    exit(1);
}

typedef struct {
    int socket;
    int index;
    char name[BUFFER_SIZE];
} client_info;

int clients[MAX_CLIENTS];
char client_names[MAX_CLIENTS][BUFFER_SIZE];
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void get_current_time(char *buffer, size_t size) {
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(buffer, size, "%Y-%m-%d %H:%M:%S", timeinfo);
}

void broadcast_message(char *message, int exclude_index, char *sender_name) {
    char buffer[BUFFER_SIZE];
    char time_buffer[BUFFER_SIZE];
    get_current_time(time_buffer, sizeof(time_buffer));

    snprintf(buffer, sizeof(buffer), "[%s] %s: %s", time_buffer, sender_name, message);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1 && i != exclude_index) {
            write(clients[i], buffer, strlen(buffer));
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void send_to_one(char *message, char *receiver_name, char *sender_name) {
    char buffer[BUFFER_SIZE];
    char time_buffer[BUFFER_SIZE];
    get_current_time(time_buffer, sizeof(time_buffer));

    snprintf(buffer, sizeof(buffer), "[%s] %s: %s", time_buffer, sender_name, message);

    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i] != -1 && strcmp(client_names[i], receiver_name) == 0) {
            write(clients[i], buffer, strlen(buffer));
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

void *handle_client(void *arg) {
    client_info *client = (client_info *)arg;
    char buffer[BUFFER_SIZE];
    int n;

    // Read the client name
    bzero(client->name, BUFFER_SIZE);
    n = read(client->socket, client->name, BUFFER_SIZE);
    if (n < 0) {
        error("Error reading from socket");
    }
    printf("Client %d name: %s\n", client->index, client->name);

    pthread_mutex_lock(&clients_mutex);
    strcpy(client_names[client->index], client->name);
    pthread_mutex_unlock(&clients_mutex);
    clock_t start, current;
    start = clock();

    while (1) {
        current = clock();
        double time_passed = ((double) (current - start)) / CLOCKS_PER_SEC;

        if (time_passed > 3600.0) { // 10.0 oznacza 10 sekund
            bzero(buffer, BUFFER_SIZE);
            write(client->socket, "ping", 4);
            sleep(2);
            n = read(client->socket, buffer, BUFFER_SIZE);
            if (strcmp(buffer, "pong") != 0) {
                pthread_mutex_lock(&clients_mutex);
                printf("Client disconnected, socket fd is %d, slot is %d\n", client->socket, client->index);
                clients[client->index] = -1;
                bzero(client_names[client->index], BUFFER_SIZE);
                pthread_mutex_unlock(&clients_mutex);
                break;
            }
            start = clock();
        }
        bzero(buffer, BUFFER_SIZE);
        n = read(client->socket, buffer, BUFFER_SIZE);
        if (n < 0) {
            error("Error reading from socket");
            break;
        } else if (n == 0) {
            printf("Client disconnected, socket fd is %d, slot is %d\n", client->socket, client->index);
            break;
        } else {
            buffer[strcspn(buffer, "\n")] = '\0';
            printf("Received from %s in slot %d: %s\n", client->name, client->index, buffer);
            char *words[MAX_WORDS];
            int word_count = 0;

            char *token = strtok(buffer, " ");
            while (token != NULL && word_count < MAX_WORDS) {
                words[word_count] = strdup(token); // Allocate memory and copy token to array
                word_count++;
                token = strtok(NULL, " ");
            }

            if (word_count > 0 && strcmp(words[0], "LIST") == 0) {
                char list_buffer[BUFFER_SIZE];
                bzero(list_buffer, BUFFER_SIZE);

                pthread_mutex_lock(&clients_mutex);
                strcat(list_buffer, "Active clients:\n");
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (clients[i] != -1) {
                        strcat(list_buffer, client_names[i]);
                        strcat(list_buffer, "\n");
                    }
                }
                pthread_mutex_unlock(&clients_mutex);

                write(client->socket, list_buffer, strlen(list_buffer));
            } else if (word_count > 1 && strcmp(words[0], "2ALL") == 0) {
                char message[BUFFER_SIZE];
                strcpy(message, words[1]);
                broadcast_message(message, client->index, client->name);
            } else if (word_count > 2 && strcmp(words[0], "2ONE") == 0) {
                char message[BUFFER_SIZE];
                char receiver_name[BUFFER_SIZE];
                strcpy(message, words[1]);
                strcpy(receiver_name, words[2]);
                send_to_one(message, receiver_name, client->name);
            } else if (strcmp(words[0], "STOP")==0){
                pthread_mutex_lock(&clients_mutex);
                write(client->socket, "Goodbye!", 8);
                printf("Client disconnected, socket fd is %d, slot is %d\n", client->socket, client->index);
                clients[client->index] = -1;
                bzero(client_names[client->index], BUFFER_SIZE);
                pthread_mutex_unlock(&clients_mutex);
            } else {
                write(client->socket, buffer, strlen(buffer));
            }

            // Free the duplicated tokens
            for (int i = 0; i < word_count; i++) {
                free(words[i]);
            }
        }
    }

    close(client->socket);
    pthread_mutex_lock(&clients_mutex);
    clients[client->index] = -1;
    bzero(client_names[client->index], BUFFER_SIZE);
    pthread_mutex_unlock(&clients_mutex);
    free(client);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <server_address> <port>\n", argv[0]);
        exit(1);
    }

    int server_socket, client_socket, port_number;
    socklen_t client_len;
    struct sockaddr_in server_addr, client_addr;

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0)
        error("Error opening socket");

    bzero((char *) &server_addr, sizeof(server_addr));
    port_number = atoi(argv[2]);

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_number);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        error("Error on binding");

    listen(server_socket, MAX_CLIENTS);
    printf("Server listening on port %d...\n", port_number);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i] = -1; 
    }

    while (1) {
        client_len = sizeof(client_addr);
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_len);
        if (client_socket < 0)
            error("Error on accept");

        pthread_mutex_lock(&clients_mutex);
        int slot = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i] == -1) {
                slot = i;
                clients[i] = client_socket;
                break;
            }
        }
        pthread_mutex_unlock(&clients_mutex);

        if (slot == -1) {
            printf("Max number of clients reached. Closing connection.\n");
            close(client_socket);
        } else {
            printf("Client connected, socket fd is %d, slot is %d\n", client_socket, slot);

            client_info *client = (client_info *)malloc(sizeof(client_info));
            client->socket = client_socket;
            client->index = slot;

            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, (void *)client);
            pthread_detach(tid);
        }
    }

    close(server_socket);
    return 0;
}
