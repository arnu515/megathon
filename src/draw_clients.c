#include <stdlib.h>
#include "raylib.h"

typedef struct client {
  struct client *next;
  int posX;
  int posY;
} Client;

Client root = {0};

void add_client(int x, int y) {
  Client *c = calloc(1, sizeof(Client));
  c->posX = x;
  c->posY = y;
  c->next = root.next;
  root.next = c;
}

void free_clients(Client *curr) {
  if (curr == NULL) return;
  free_clients(curr->next);
  free(curr);
}

// Assuming BeginDrawing.
void draw_clients() {
  for (Client *curr = root.next; curr != NULL; curr = curr->next) {
    DrawRectangle(curr->posX, curr->posY, 50, 50, BLUE);
  }
}
