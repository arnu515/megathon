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
int connect_to_server(struct addrinfo *servinfo, int *sockfd, char *s);
int receive_data(int sockfd, char *buf);

// Function definitions
#ifndef _CLIENT_IMPLEMENTATION

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

int connect_to_server(struct addrinfo *servinfo, int *sockfd, char *s)
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

int receive_data(int sockfd, char *buf)
{
    int numbytes;
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        perror("recv");
        return 1;
    }
    buf[numbytes] = '\0';
    printf("client: received '%s'\n", buf);
    return 0;
}

#endif
#endif