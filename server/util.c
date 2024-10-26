#include <arpa/inet.h>
#include <stdio.h>

int write_num(int sockfd, ssize_t num) {
  char str[20];

  int n = snprintf(str, sizeof(str), "%zd", num);
  return send(sockfd, str, n, 0);
}

int write_client_posn(int sockfd, int clientfd, int x, int y) {
  char str[40];

  int n = snprintf(str, sizeof(str), "%d-(%d,%d)", clientfd, x, y);
  return send(sockfd, str, n, 0);
}
