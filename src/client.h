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

#define PORT "8080"       // the port client will be connecting to 
#define MAXDATASIZE 100   // max number of bytes we can get at once 

// Function declarations
void *get_in_addr(struct sockaddr *sa);
int setup_addrinfo(char *hostname, struct addrinfo **servinfo);
int setup_conn(struct addrinfo *servinfo, int *sockfd, char *s);
ssize_t receive_data(int sockfd, char *buf);
ssize_t send_data(int, const char *, size_t);
void connect_to_server(char *, struct addrinfo **, int *, char []);
void fetch_and_set_starting_pos(int, int *, int *, int *);
void get_clients(int, void (*)(int, int, int));
void send_pos(int sockfd, int x, int y);

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

ssize_t receive_data(int sockfd, char *buf)
{
    ssize_t numbytes;
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        return -1;
    }
    buf[numbytes] = '\0';
    printf("client: received '%s'\n", buf);
    return numbytes;
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
  if (receive_data(sockfd, buf) < 0) {
    perror("receive");
    TraceLog(LOG_FATAL, "Could not get starting coordinates");
    exit(1);
  }
  TraceLog(LOG_INFO, "Starting coords: %s", buf);
  sscanf(buf, "%d-(%d,%d)", id, x, y);
}

void get_clients(int sockfd, void (*on_new)(int, int, int)) {
  send_data(sockfd, "get", 3);
  char buf[MAXDATASIZE] = {0};
  if (receive_data(sockfd, buf) < 0) {
    perror("receive");
    TraceLog(LOG_FATAL, "Could not get other clients");
    exit(1);
  }
  long num = atol(buf);
  TraceLog(LOG_INFO, "%d clients connected\n", num);
  while (num-- > 0) {
    if (receive_data(sockfd, buf) < 0) {
      perror("receive");
      TraceLog(LOG_FATAL, "Could not get other clients");
      exit(1);
    }
    int id, x, y;
    sscanf(buf, "%d-(%d,%d)", &id, &x, &y);
    printf("Got client: x=%d, y=%d", x, y);
    on_new(id, x, y);
  }
}

void send_pos(int sockfd, int x, int y) {
  char buf[32] = {0};
  int n = snprintf(buf, 31, "pos-(%d,%d)", x, y);
  send_data(sockfd, buf, n);
}

#endif
#endif
