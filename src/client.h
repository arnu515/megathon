#ifndef CLIENT_H
#define CLIENT_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef PORT
  #define PORT "8080"       // the port client will be connecting to 
#endif
#define MAXDATASIZE 100   // max number of bytes we can get at once 

// Function declarations
void *get_in_addr(struct sockaddr *sa);
int setup_addrinfo(char *hostname, struct addrinfo **servinfo);
int setup_conn(struct addrinfo *servinfo, int *sockfd, char *s);
ssize_t receive_data(int sockfd, char *buf, int flags);
ssize_t send_data(int, const char *, size_t);
void connect_to_server(char *, struct addrinfo **, int *, char []);
void fetch_and_set_starting_pos(int, int *, int *, int *);
void fetch_and_set_starting_candies(int, int *);
void fetch_and_set_starting_prompt(int, int *);
void get_clients(int, void (*)(int, int, int));
void send_pos(int sockfd, int x, int y);
void send_pumpkin(int, int);
void send_candies(int, int);

// Function definitions
#ifdef _CLIENT_IMPLEMENTATION

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int setup_addrinfo(char *hostname, struct addrinfo **servinfo)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int rv;
    if ((rv = getaddrinfo(hostname, PORT, &hints, servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    return 0;
}

int setup_conn(struct addrinfo *servinfo, int *sockfd, char *s)
{
    struct addrinfo *p;
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(*sockfd);
            perror("client: connect");
            continue;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, INET6_ADDRSTRLEN);
        printf("client: connecting to %s\n", s);
        return 0;
    }

    fprintf(stderr, "client: failed to connect\n");
    return 2;
}

char receive_buf[MAXDATASIZE] = {0};

ssize_t receive_data(int sockfd, char *buf, int flags) {
  ssize_t bytes_received;
  
  if (*receive_buf != '\0') { // there's data in the buffer
    char *newline_pos = strchr(receive_buf, '\n');
    printf("recccc: %s %p\n", receive_buf, newline_pos);
    if (newline_pos != NULL) {
      *newline_pos = '\0';
      strcpy(buf, receive_buf);
      memmove(receive_buf, newline_pos + 1, strlen(newline_pos + 1) + 1);
      printf("check recv buf %d: %s\n", strlen(buf), buf);
      return strlen(buf);
    }
  }

  memset(receive_buf, 0, MAXDATASIZE);
  
  bytes_received = recv(sockfd, buf, MAXDATASIZE - 1, flags);
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

ssize_t send_data(int sockfd, const char *buf, size_t len) {
    ssize_t n;
    if ((n = send(sockfd, buf, len, 0)) < 0) {
        perror("send");
        return -1;
    }
    return n;
}

void connect_to_server(char *host, struct addrinfo **servinfo, int *sockfd, char s[]) {
  if (setup_addrinfo(host, servinfo) != 0) {
    TraceLog(LOG_FATAL, "Could not setup address");
    exit(1);
  }

  if (setup_conn(*servinfo, sockfd, s) != 0) {
    TraceLog(LOG_FATAL, "Could not connect to server");
    freeaddrinfo(*servinfo);
    exit(1);
  }

  freeaddrinfo(*servinfo);
}

void fetch_and_set_starting_pos(int sockfd, int *id, int *x, int *y) {
  char buf[MAXDATASIZE] = {0};
  if (receive_data(sockfd, buf, 0) < 0) {
    perror("receive");
    TraceLog(LOG_FATAL, "Could not get starting coordinates");
    exit(1);
  }
  TraceLog(LOG_INFO, "Starting coords: %s", buf);
  sscanf(buf, "%d-(%d,%d)", id, x, y);
}

void fetch_and_set_starting_candies(int sockfd, int *candies) {
  char buf[MAXDATASIZE] = {0};
  if (receive_data(sockfd, buf, 0) < 0) {
    perror("receive");
    TraceLog(LOG_FATAL, "Could not get candies");
    exit(1);
  }
  *candies = atoi(buf);
  TraceLog(LOG_INFO, "Starting with %d candies.", *candies);
}

void fetch_and_set_starting_prompt(int sockfd, int *prompt) {
  char buf[MAXDATASIZE] = {0};
  if (receive_data(sockfd, buf, 0) < 0) {
    perror("receive");
    TraceLog(LOG_FATAL, "Could not get prompt");
    exit(1);
  }
  *prompt = atoi(buf);
  TraceLog(LOG_INFO, "Starting with prompt %d.", *prompt);
}

void get_clients(int sockfd, void (*on_new)(int, int, int)) {
  send_data(sockfd, "get\n", 3);
  char buf[MAXDATASIZE] = {0};
  if (receive_data(sockfd, buf, 0) < 0) {
    perror("receive");
    TraceLog(LOG_FATAL, "Could not get other clients");
    exit(1);
  }
  long num = atol(buf);
  TraceLog(LOG_INFO, "%d clients connected\n", num);
  while (num-- > 0) {
    if (receive_data(sockfd, buf, 0) < 0) {
      perror("receive");
      TraceLog(LOG_FATAL, "Could not get other clients");
      exit(1);
    }
    printf("EEEEEEEEEERecv: %s\n", buf);
    int id, x, y;
    sscanf(buf, "%d-(%d,%d)", &id, &x, &y);
    printf("Got client %d: x=%d, y=%d\n", id, x, y);
    on_new(id, x, y);
  }
}

void send_pos(int sockfd, int x, int y) {
  char buf[32] = {0};
  int n = snprintf(buf, 31, "pos-(%d,%d)\n", x, y);
  send_data(sockfd, buf, n);
}

void send_pumpkin(int sockfd, int idx) {
  char buf[32] = {0};
  int n = snprintf(buf, 31, "pumpkin-(%d)\n", idx);
  send_data(sockfd, buf, n);
}

void send_candies(int sockfd, int candies) {
  char buf[32] = {0};
  int n = snprintf(buf, 31, "candies-(%d)\n", candies);
  send_data(sockfd, buf, n);
}

#endif
#endif
