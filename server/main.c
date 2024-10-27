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

#ifndef PORT
    #define PORT 8080
#endif
#define BUFFER_SIZE 1024
#define MAX_CLIENTS 10
#define START_POS 456
#define START_CANDIES 0
#define MAXDATASIZE 100

typedef struct client {
    int posX;
    int posY;
    int socket;
    int candies;
} Client;

Client client_sockets[MAX_CLIENTS];
int client_count = 0;
pthread_mutex_t client_mutex = PTHREAD_MUTEX_INITIALIZER;
int current_prompt = 1;

void broadcast_pos(int sender_socket, char data[]) {
    int x, y;
    sscanf(data, "pos-(%d,%d)", &x, &y);
    char buf[MAXDATASIZE];
    int n = snprintf(buf, MAXDATASIZE-1, "pos_%d-(%d,%d)\n", sender_socket, x, y);
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i].socket != sender_socket) {
            send(client_sockets[i].socket, buf, n, 0);
        } else {
            client_sockets[i].posX = x;
            client_sockets[i].posY = y;
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void broadcast_pumpkin(int sender_socket, char data[]) {
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) 
        if (client_sockets[i].socket != sender_socket) send(client_sockets[i].socket, data, strlen(data), 0);
    pthread_mutex_unlock(&client_mutex);
}

int cmp_place(const void *a, const void *b) {
    return ((const int *)b)[0] - ((const int *)a)[0];
}

void send_place(int sender_socket) {
    pthread_mutex_lock(&client_mutex);
    int candy_socks[client_count][2];
    for (int i = 0; i < client_count; i++) {
        candy_socks[i][0] = client_sockets[i].candies;
        candy_socks[i][1] = client_sockets[i].socket;
    }
    qsort(candy_socks, client_count, sizeof(int[2]), cmp_place);
    for (int i = 0; i < client_count; i++) {
      char str[20];

      int n = snprintf(str, sizeof(str), "place-(%d)\n", i+1);
      send(candy_socks[i][1], str, n, 0);
    }
    pthread_mutex_unlock(&client_mutex);
}

void set_candies(int sender_socket, char data[]) {
    int candies;
    sscanf(data, "candies-(%d)", &candies);
    if (candies < 0) {
        printf("Client %d has lost the game (candies: %d)", sender_socket, candies);
        send(sender_socket, "lose", 4, 0);
        return;
    }
    pthread_mutex_lock(&client_mutex);
    for (int i = 0; i < client_count; i++) {
        if (client_sockets[i].socket == sender_socket) {
            client_sockets[i].candies = candies;
        }
    }
    pthread_mutex_unlock(&client_mutex);
}

void client_join_leave(bool is_join, int sender_socket) {
    char buf[20];
    int n = is_join ? snprintf(buf, sizeof(buf), "join_%d-(%d,%d)\n", sender_socket, START_POS, START_POS) : snprintf(buf, sizeof(buf), "leave_%d\n", sender_socket);
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
            printf("Sending client %d (%d,%d) to %d\n", client_sockets[i].socket, client_sockets[i].posX, client_sockets[i].posY, socket);
            write_client_posn(socket, client_sockets[i].socket, client_sockets[i].posX, client_sockets[i].posY);
        }
    }
    pthread_mutex_unlock(&client_mutex);
}


char receive_buf[MAXDATASIZE] = {0};

ssize_t receive_data(int sockfd, char *buf, size_t size) {
  ssize_t bytes_received;
  
  if (*receive_buf != '\0') { // there's data in the buffer
    char *newline_pos = strchr(receive_buf, '\n');
    printf("recccc: %s %p\n", receive_buf, newline_pos);
    if (newline_pos != NULL) {
      *newline_pos = '\0';
      strcpy(buf, receive_buf);
      memmove(receive_buf, newline_pos + 1, strlen(newline_pos + 1) + 1);
      printf("check recv buf %ld: %s\n", strlen(buf), buf);
      return strlen(buf);
    }
  }

  memset(receive_buf, 0, MAXDATASIZE);
  
  bytes_received = read(sockfd, buf, size);
  if (bytes_received == -1) {
    perror("recv");
    return -1;
  }
  
  buf[bytes_received] = '\0';
  
  char *newline_pos = strchr(buf, '\n');
  if (newline_pos != NULL) {
    *newline_pos = '\0';
    memmove(receive_buf, newline_pos + 1, strlen(newline_pos + 1) + 1);
    printf("recv buf: %s\n", buf);
  }
  printf("client: received '%s'\n", buf);
  return bytes_received;
}

void *handle_client(void *arg) {
    int new_socket = *(int *)arg;
    char buffer[BUFFER_SIZE] = {0};

    client_join_leave(true, new_socket);

    while (1) {
        int valread = receive_data(new_socket, buffer, BUFFER_SIZE);
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

        if (strncmp(buffer, "candies-", 8) == 0) {
            set_candies(new_socket, buffer);
            continue;
        }

        if (strncmp(buffer, "pumpkin-", 8) == 0) {
            broadcast_pumpkin(new_socket, buffer);
            continue;
        }

        if (strncmp(buffer, "get", 3) == 0) {
            send_all_coords(new_socket);
            continue;
        }

        if (strncmp(buffer, "end", 3) == 0) {
            send_place(new_socket);
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

    srand(time(NULL));
    
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
        if (client_count == 0) current_prompt = rand() % 2 + 1;

        pthread_mutex_lock(&client_mutex);
        if (client_count < MAX_CLIENTS) {
            client_sockets[client_count].posX = START_POS;
            client_sockets[client_count].posY = START_POS;
            client_sockets[client_count].candies = START_CANDIES;
            client_sockets[client_count++].socket = new_socket;
            write_client_posn(new_socket, new_socket, START_POS, START_POS);
            write_num(new_socket, START_CANDIES);
            write_num(new_socket, current_prompt);
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
