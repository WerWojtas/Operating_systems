#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

int client_socket;
pthread_mutex_t socket_mutex;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void handle_sigint(int sig) {
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    strcpy(buffer, "DISCONNECT");

    pthread_mutex_lock(&socket_mutex);
    if (write(client_socket, buffer, strlen(buffer)) < 0) {
        perror("Error writing to socket");
    }
    pthread_mutex_unlock(&socket_mutex);

    close(client_socket);
    printf("\nClient disconnected.\n");
    pthread_mutex_destroy(&socket_mutex);
    exit(0);
}

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    int n;
    while (1) {
        bzero(buffer, BUFFER_SIZE);
        
        pthread_mutex_lock(&socket_mutex);
        n = read(client_socket, buffer, BUFFER_SIZE);
        pthread_mutex_unlock(&socket_mutex);

        if (n < 0)
            error("Error reading from socket");
        else if (n == 0) {
            printf("Server closed the connection\n");
            break;
        }
        if (strncmp(buffer, "Goodbye!", 8) == 0) {
            printf("Server disconnected\n");
            close(client_socket);
            exit(0);
        }
        if (strncmp(buffer, "ping", 4) == 0) {
            pthread_mutex_lock(&socket_mutex);
            write(client_socket, "pong", 4);
            pthread_mutex_unlock(&socket_mutex);
            continue;
        }
        printf("Received: %s\n", buffer);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s <server_ip> <port> <client_name>\n", argv[0]);
        exit(1);
    }

    char *server_ip = argv[1];
    int port_number = atoi(argv[2]);
    char *client_name = argv[3];

    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    signal(SIGINT, handle_sigint);

    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0)
        error("Error opening socket");

    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port_number);

    if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
        error("Invalid address");

    if (connect(client_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        error("Connection failed");

    printf("Connected to server\n");
    char result[50];
    if (snprintf(result, sizeof(result), "%s %s", "HELLO", client_name) < 0)
        error("Error creating message");
    printf("Sending: %s\n", result);
    int n = write(client_socket, result, strlen(result));
    if (n < 0)
        error("Error writing to socket");

    pthread_mutex_init(&socket_mutex, NULL);

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        fprintf(stderr, "Error creating receive thread\n");
        return 1;
    }

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);

        pthread_mutex_lock(&socket_mutex);
        n = write(client_socket, buffer, strlen(buffer));
        pthread_mutex_unlock(&socket_mutex);

        if (n < 0)
            error("Error writing to socket");
    }

    pthread_cancel(recv_thread);
    close(client_socket);
    pthread_mutex_destroy(&socket_mutex);
    return 0;
}
