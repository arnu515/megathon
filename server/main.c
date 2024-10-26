#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10

typedef struct client {
    int posX;
    int posY;
    int socket;
} Client;

Client client_sockets[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(const char *message, int sender_socket) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i].socket != sender_socket) {
            send(client_sockets[i].socket, message, strlen(message), 0);
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void *handle_client(void *arg) {
    int new_socket = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};

    while (1) {
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            perror("read");
            break; // Exit the loop on read error
        }

        // Null-terminate the buffer to treat it as a string
        buffer[valread] = '\0';

        printf("Received: %s\n", buffer);

        // Check if the client sent "close"
        if (strncmp(buffer, "close", 5) == 0) {
            printf("Closing connection as requested by client.\n");
            break; // Exit the loop to close the connection
        }

        // Check if the client sent "broadcast"
        if (strncmp(buffer, "broadcast", 9) == 0) {
            broadcast_message("Hello world\n", new_socket);
            continue; // Skip echoing back
        }

        // if (strncmp(buffer, "get", ))

        // Echoing back the received data
        send(new_socket, buffer, valread, 0);
        printf("Echoed back: %s\n", buffer);
    }

    // Close the connection and remove the socket from the list
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
            memset(&client_sockets[client_count++], 0, sizeof(Client));
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
