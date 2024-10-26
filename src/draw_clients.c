#include <stdbool.h>
#include <stdlib.h>
#include "raylib.h"

typedef struct client {
  struct client *next;
  int id;
  int posX;
  int posY;
} Client;

Client root = {0};

void add_client(int id, int x, int y) {
  Client *c = calloc(1, sizeof(Client));
  c->id   = id;
  c->posX = x;
  c->posY = y;
  c->next = root.next;
  root.next = c;
}

bool update_client(int id, int x, int y) {
  for (Client *curr = root.next; curr != NULL; curr = curr->next) {
    if (curr->id == id) {
      curr->posX = x;
      curr->posY = y;
      return true;
    } 
  }
  return false;
}

bool remove_client(int id) {
  Client *prev = &root;
  for (Client *curr = root.next; curr != NULL; curr = curr->next) {
    if (curr->id == id) {
      prev->next = curr->next;
      free(curr);
      return true;
    } 
    prev = curr;
  }
  return false;
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
