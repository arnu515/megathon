#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "util.c"

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define START_POS 456
#define MAXDATASIZE 100

typedef struct client {
    int posX;
    int posY;
    int socket;
} Client;

Client client_sockets[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_pos(int sender_socket, char data[]) {
    int x, y;
    sscanf(data, "pos-(%d,%d)", &x, &y);
    char buf[MAXDATASIZE];
    int n = snprintf(buf, MAXDATASIZE-1, "pos_%d-(%d,%d)", sender_socket, x, y);
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i].socket != sender_socket) {
            send(client_sockets[i].socket, buf, n, 0);
        }
    }
    pthread_mutex_unlock(&client_mutex);
}
void client_join_leave(bool is_join, int sender_socket) {
    char buf[20];
    int n = snprintf(buf, sizeof(buf), is_join ? "join_%d" : "leave_%d", sender_socket);
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i].socket != sender_socket) {
            send(client_sockets[i].socket, buf, n, 0);
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void send_all_coords(int socket) {
    write_num(socket, client_count-1);
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i].socket != socket) {
            write_client_posn(socket, client_sockets[i].socket, client_sockets[i].posX, client_sockets[i].posY);
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void *handle_client(void *arg) {
    int new_socket = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};

    client_join_leave(true, new_socket);

    while (1) {
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            perror("read");
            break; // Exit the loop on read error
        }

        // Null-terminate the buffer to treat it as a string
        buffer[valread] = '\0';

        printf("Received: %s\n", buffer);

        if (strncmp(buffer, "close", 5) == 0) {
            printf("Closing connection as requested by client.\n");
            break;
        }

        if (strncmp(buffer, "pos-", 4) == 0) {
            broadcast_pos(new_socket, buffer);
            continue;
        }

        if (strncmp(buffer, "get", 3) == 0) {
            send_all_coords(new_socket);
            continue;
        }

        // Echoing back the received data
        send(new_socket, buffer, valread, 0);
        printf("Echoed back: %s\n", buffer);
    }

    // Close the connection and remove the socket from the list
    printf("Client %d disconnected\n", new_socket);
    client_join_leave(false, new_socket);
    close(new_socket);
    
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i].socket == new_socket) {
            client_sockets[i] = client_sockets[client_count - 1]; // Replace with last socket
            client_count--;
            break;
        }
    }
    pthread_mutex_unlock(&client_mutex);

    return NULL;
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    
    memset(client_sockets, 0, sizeof(client_sockets));

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // Any incoming interface
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Echo server is listening on port %d\n", PORT);

    while (1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            continue;
        }

        printf("New connection established %d\n", client_count);

        pthread_mutex_lock(&client_mutex);
        if (client_count < MAX_CLIENTS) {
            client_sockets[client_count].posX = START_POS;
            client_sockets[client_count].posY = START_POS;
            client_sockets[client_count++].socket = new_socket;
            write_client_posn(new_socket, new_socket, START_POS, START_POS);
        } else {
            printf("Max clients reached. Connection rejected.\n");
            close(new_socket);
        }
        pthread_mutex_unlock(&client_mutex);

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)&new_socket) != 0) {
            perror("pthread_create failed");
            close(new_socket);
        }
        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}
