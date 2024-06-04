#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>

#define BUFFER_SIZE 1024

int client_socket;

void error(const char *msg) {
    perror(msg);
    exit(1);
}

void handle_sigint(int sig) {
    char buffer[BUFFER_SIZE];
    bzero(buffer, BUFFER_SIZE);
    strcpy(buffer, "DISCONNECT");

    // Inform the server that the client is disconnecting
    if (write(client_socket, buffer, strlen(buffer)) < 0) {
        perror("Error writing to socket");
    }

    close(client_socket);
    printf("\nClient disconnected.\n");
    exit(0);
}

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    int n;
    while (1) {
        bzero(buffer, BUFFER_SIZE);
        n = read(client_socket, buffer, BUFFER_SIZE);
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
            write(client_socket, "pong", 4);
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
    int n = write(client_socket, client_name, strlen(client_name));
    if (n < 0)
        error("Error writing to socket");

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        fprintf(stderr, "Error creating receive thread\n");
        return 1;
    }

    while (1) {
        bzero(buffer, BUFFER_SIZE);
        fgets(buffer, BUFFER_SIZE, stdin);

        n = write(client_socket, buffer, strlen(buffer));
        if (n < 0)
            error("Error writing to socket");
    }

    pthread_cancel(recv_thread);
    close(client_socket);
    return 0;
}
