#include "../raylib/include/raylib.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>

#include "draw_clients.c"

#define _CLIENT_IMPLEMENTATION
#include "client.h"

struct addrinfo *servinfo;
int sockfd;
char s[INET6_ADDRSTRLEN];

#define HOST "127.0.0.1"

void listen_for_data(int socket) {
  char buffer[MAXDATASIZE];
  ssize_t bytes_received;
  // Receive data from the server in a non-blocking way
  fd_set read_fds;
  FD_ZERO(&read_fds);
  FD_SET(socket, &read_fds);

  struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 100000; // 100 ms

  if (select(socket + 1, &read_fds, NULL, NULL, &timeout) > 0) {
    if (FD_ISSET(socket, &read_fds)) {
      bytes_received = receive_data(socket, buffer, MSG_DONTWAIT);
      if (bytes_received > 0) {
        // Print the received data to the console
        buffer[bytes_received] = '\0';
        if (strncmp(buffer, "join_", 5) == 0) {
          int nid, x, y;
          sscanf(buffer, "join_%d-(%d,%d)", &nid, &x, &y);
          add_client(nid, x, y);
        } else if (strncmp(buffer, "leave_", 5) == 0) {
          int nid, x, y;
          sscanf(buffer, "leave_%d", &nid);
          remove_client(nid);
        } else if (strncmp(buffer, "pos_", 4) == 0) {
          int nid, x, y;
          sscanf(buffer, "pos_%d-(%d,%d)", &nid, &x, &y);
          update_client(nid, x, y);
        }
        printf("Received: %s\n", buffer);
      }
      else if (bytes_received == 0)  // Connection closed by the server
        TraceLog(LOG_INFO, "Connection closed by the server");
      else // Error while receiving data
        TraceLog(LOG_ERROR, "Error while receiving data: %d", errno);
    }
  }
}

#define CANDY_X 20
#define CANDY_Y 20

void draw_candies(Texture2D sprite, int candies) {
  char buf[20];
  DrawTexture(sprite, CANDY_X, CANDY_Y, WHITE);
  DrawText(buf, CANDY_X + 20, CANDY_Y, 10, WHITE);
}

#define WALL_WIDTH 50
#define WALL_HEIGHT 50

#define GHOST_WIDTH 25
#define GHOST_HEIGHT 25

#define NUM_GHOSTS 5  // Number of ghosts


typedef struct Ghost {
    int x;
    int y;
    int startX;    // Starting X position
    int startY;    // Starting Y position
    int targetX;   // Target X position
    int targetY;   // Target Y position
    int speed;
} Ghost;


Ghost ghosts[NUM_GHOSTS] = {
    {150, 150, 150, 150, 450, 150, 3},
    {200, 150, 200, 150, 400, 150, 3},
    {150, 200, 150, 200, 150, 400, 3}, {700, 150, 700, 150, 500, 150, 3},
    {150, 650, 150, 650, 150, 500, 3},
};


// Struct to hold wall positions
typedef struct Wall {
    int x;
    int y;
} Wall;

// Struct to hold pumpkin positions and visibility status
typedef struct Pumpkin {
    int x;
    int y;
    bool isVisible;  // New flag for visibility
} Pumpkin;

Wall walls[] = {

    // Top border wall sections
    {0, 0}, {50, 0}, {100, 0}, {150, 0}, {200, 0}, {250, 0}, {300, 0}, {350, 0}, {400, 0}, {450, 0}, {500, 0}, {550, 0}, {600, 0}, {650, 0}, {700, 0}, {750, 0}, {800, 0}, {850, 0}, {900, 0},

    // Left border wall sections
    {0, 50}, {0, 100}, {0, 150}, {0, 200}, {0, 250}, {0, 300}, {0, 350}, {0, 400}, {0, 450}, {0, 500}, {0, 550}, {0, 600}, {0, 650}, {0, 700}, {0, 750}, {0, 800}, {0, 850},

    // Right border wall sections
    {900, 50}, {900, 100}, {900, 150}, {900, 200}, {900, 250}, {900, 300}, {900, 350}, {900, 400}, {900, 450}, {900, 500}, {900, 550}, {900, 600}, {900, 650}, {900, 700}, {900, 750}, {900, 800}, {900, 850},

    // Bottom border wall sections
    {0, 900}, {50, 900}, {100, 900}, {150, 900}, {200, 900}, {250, 900}, {300, 900}, {350, 900}, {400, 900}, {450, 900}, {500, 900}, {550, 900}, {600, 900}, {650, 900}, {700, 900}, {750, 900}, {800, 900}, {850, 900}, {900, 900},
    {450, 0}, {450, 50}, {450, 100},
    
    {450, 150}, {450, 200}, {450, 250}, //top wala 

    {450, 900}, {450, 850}, {450, 800},
    {450, 750}, {450, 700}, {450, 650}, //bottom wala

    //CENTRAL DOUBLES
    {450, 350}, {450, 550},
    {350, 450}, {550, 450},

    //DIAGONALS (TL-BR)
    {50, 50}, {100, 100}, {150, 150}, {200, 200}, {250, 250}, {300, 300},
    {600, 600}, {650, 650}, {700, 700}, {750, 750}, {800, 800}, {850, 850},

    //RIGHT
    {650, 450}, {700, 450}, //{700, 350}, {700, 400}, {700, 550}, {700, 500},
    {736, 290}, {736, 340}, {736, 390}, {736, 440}, {736, 490}, {736, 540}, {736, 590}, {800, 450},

    //LEFT
    {50, 450}, {100, 450}, {100, 350}, {100, 300}, {100, 400}, {100, 600}, {100, 550}, {100, 500},
    {150, 450}, {200, 450}, {250, 450},

    //DIAGONALS (TR-BL)
    {775,75},{825,25},{725,125},{675,175},{625,225},{575,275},{275,575},{225,625},{175, 675},{125,725},{75,775},{25,825}


};

Pumpkin pumpkins[] = {
    {150, 50, true},  // Set initial visibility to true
    {350, 50, true},
    {550, 50, true},
    {750, 50, true},
    {50, 150, true},
    {50, 350, true},
    {50, 550, true},
    {50, 750, true},
    {150, 800, true},
    {350, 800, true},
    {550, 800, true},
    {750, 800, true},
    {775, 150, true},
    {775, 350, true},
    {775, 550, true},
    {775, 750, true},
};

// Function to check collision between player and walls
bool CheckCollisionWithWalls(int x, int y) {
    Rectangle playerRect = {x, y, 25, 25};  // Player's bounding box
    for (int i = 0; i < sizeof(walls) / sizeof(Wall); i++) {
        Rectangle wallRect = {walls[i].x, walls[i].y, WALL_WIDTH, WALL_HEIGHT};
        if (CheckCollisionRecs(playerRect, wallRect)) {
            return true;  // Collision detected
        }
    }
    return false;  // No collision
}

// Function to check collision between player and pumpkins
void CheckCollisionWithPumpkins(int x, int y, int *score) {
    Rectangle playerRect = {x, y, 25, 25};  // Player's bounding box
    for (int i = 0; i < sizeof(pumpkins) / sizeof(Pumpkin); i++) {
        if (pumpkins[i].isVisible) {  // Only check visible pumpkins
            Rectangle pumpkinRect = {pumpkins[i].x, pumpkins[i].y, WALL_WIDTH, WALL_HEIGHT};
            if (CheckCollisionRecs(playerRect, pumpkinRect)) {
                pumpkins[i].isVisible = false;  // Hide the pumpkin
                (*score)++;  // Increment score
            }
        }
    }
}
bool CheckCollisionWithAnyGhost(int playerX, int playerY, Ghost ghosts[], int numGhosts) {
    Rectangle playerRect = {playerX, playerY, 25, 25};
    for (int i = 0; i < numGhosts; i++) {
        Rectangle ghostRect = {ghosts[i].x, ghosts[i].y, GHOST_WIDTH, GHOST_HEIGHT};
        if (CheckCollisionRecs(playerRect, ghostRect)) return true;
    }
    return false;
}


// Function to move the ghost between its predefined coordinates
// Function to move the ghosts between predefined coordinates
void MoveGhosts(Ghost ghosts[], int numGhosts) {
    for (int i = 0; i < numGhosts; i++) {
        // Move along x-axis
        if (ghosts[i].x < ghosts[i].targetX) ghosts[i].x += ghosts[i].speed;
        else if (ghosts[i].x > ghosts[i].targetX) ghosts[i].x -= ghosts[i].speed;

        // Move along y-axis
        if (ghosts[i].y < ghosts[i].targetY) ghosts[i].y += ghosts[i].speed;
        else if (ghosts[i].y > ghosts[i].targetY) ghosts[i].y -= ghosts[i].speed;

        // Check if the ghost has reached its target position
        if (ghosts[i].x == ghosts[i].targetX && ghosts[i].y == ghosts[i].targetY) {
            // Swap the target and starting positions
            int tempX = ghosts[i].targetX;
            int tempY = ghosts[i].targetY;
            ghosts[i].targetX = ghosts[i].startX;
            ghosts[i].targetY = ghosts[i].startY;
            ghosts[i].startX = tempX;
            ghosts[i].startY = tempY;
        }
    }
}





int main(void) {
    InitWindow(900, 900, "YAAAS");
  BeginDrawing();
    ClearBackground((Color){0, 0, 0, 0});
    DrawText("connecting...", 456, 456, 20, RAYWHITE);
  EndDrawing();
    
    Rectangle sourceRec = { 0, 0, 50, 50 };
    Texture2D sprite = LoadTexture("./Finals/Girl_Final.png");
    Texture2D wallTexture = LoadTexture("./wall/sprite.png");
    Texture2D pumpkinTexture = LoadTexture("./wall/candy.png");
    Texture2D candyTexture = LoadTexture("./wall/candy.png");
    Texture2D ghostTexture = LoadTexture("./Finals/Trader_Final.png");

  int id, x, y, candies;

  TraceLog(LOG_INFO, "Connecting to server");
  connect_to_server(HOST, &servinfo, &sockfd, s);
  TraceLog(LOG_INFO, "Getting start pos");
  fetch_and_set_starting_pos(sockfd, &id, &x, &y);
  fetch_and_set_starting_candies(sockfd, &candies);
  TraceLog(LOG_INFO, "Getting other connected clients");
  get_clients(sockfd, add_client);

  SetTargetFPS(30);

    while (!WindowShouldClose()) {
        listen_for_data(sockfd);
        int prevX = x;
        int prevY = y;

        // Update player position with boundary checks
        if (IsKeyDown(KEY_W) && y > wallTexture.height) y -= 10;
        if (IsKeyDown(KEY_A) && x > wallTexture.width) x -= 10;
        if (IsKeyDown(KEY_S) && y < GetScreenHeight() - wallTexture.height - sourceRec.height) y += 10;
        if (IsKeyDown(KEY_D) && x < GetScreenWidth() - wallTexture.width - sourceRec.width) x += 10;

        if (CheckCollisionWithWalls(x, y)) {
            x = prevX;
            y = prevY;
        }

        // Check collisions with pumpkins
        CheckCollisionWithPumpkins(x, y, &candies);
        MoveGhosts(ghosts, NUM_GHOSTS);


        // Check if player collides with the ghost
//         if (CheckCollisionWithAnyGhost(x, y, ghosts, NUM_GHOSTS)) {
//     candies = (candies > 0) ? candies - 1 : 0;
//     if (candies == 0) {
//         DrawText("Game Over!", GetScreenWidth() / 2 - 50, GetScreenHeight() / 2, 40, RED);
//         EndDrawing();
//         break;
//     }
// }
        if (x != prevX || y != prevY) send_pos(sockfd, x, y);

        BeginDrawing();
            ClearBackground((Color){0, 0, 0, 0});
            
            // Draw the wall texture as the border
            for (int i = 0; i < GetScreenWidth() / wallTexture.width; i++) {
                DrawTexture(wallTexture, i * wallTexture.width, 0, WHITE); 
                DrawTexture(wallTexture, i * wallTexture.width, GetScreenHeight() - wallTexture.height, WHITE);
            }

            for (int i = 0; i < GetScreenHeight() / wallTexture.height; i++) {
                DrawTexture(wallTexture, 0, i * wallTexture.height, WHITE);
                DrawTexture(wallTexture, GetScreenWidth() - wallTexture.width, i * wallTexture.height, WHITE);
            }

            for (int i = 0; i < sizeof(walls) / sizeof(Wall); i++) {
                DrawTexture(wallTexture, walls[i].x, walls[i].y, WHITE);
            }

            for (int i = 0; i < sizeof(pumpkins) / sizeof(Pumpkin); i++) {
                if (pumpkins[i].isVisible) {  // Only draw visible pumpkins
                    DrawTexture(pumpkinTexture, pumpkins[i].x, pumpkins[i].y, WHITE);
                }
            }
            for (int i = 0; i < NUM_GHOSTS; i++) {
                DrawTexture(ghostTexture, ghosts[i].x, ghosts[i].y, WHITE);
            }

            // Draw player sprite
            DrawTextureRec(sprite, sourceRec, (Vector2){ x, y }, WHITE);

            // Draw candy score
          draw_candies(candyTexture, candies);

            // Draw other players
          draw_clients(sprite, sourceRec);

            //Draw ghost
            
        EndDrawing();
    }

    UnloadTexture(sprite);
    UnloadTexture(wallTexture);
    UnloadTexture(pumpkinTexture);  // Unload pumpkin texture
    CloseWindow();

  free_clients(root.next);
  return 0;
}
