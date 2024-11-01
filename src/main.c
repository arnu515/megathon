#include "../raylib/include/raylib.h"
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>

#include "draw_clients.c"

#define _CLIENT_IMPLEMENTATION
#include "client.h"

struct addrinfo *servinfo;
int sockfd;
char s[INET6_ADDRSTRLEN];

#ifndef HOST
    #define HOST "127.0.0.1"
#endif

#define CANDY_X 20
#define CANDY_Y 20
#define CANDY_WIDTH 120
#define CANDY_HEIGHT 50
#define CANDY_ALPHA 100

void draw_candies(Texture2D sprite, int candies) {
  char buf[3];
  snprintf(buf, 3, "%.2d", candies);
  DrawRectangle(CANDY_X, CANDY_Y-5, CANDY_WIDTH, CANDY_HEIGHT, (Color){0, 0, 0, CANDY_ALPHA});
  DrawTexture(sprite, CANDY_X, CANDY_Y, WHITE);
  DrawText(buf, CANDY_X + CANDY_WIDTH/1.5, CANDY_Y + CANDY_HEIGHT/5.5, 25, WHITE);
}

#define WALL_WIDTH 50
#define WALL_HEIGHT 50

#define GHOST_WIDTH 25
#define GHOST_HEIGHT 25

#define NUM_GHOSTS 9  // Number of ghosts


typedef struct Ghost {
    int x;
    int y;
    int startX;    // Starting X position
    int startY;    // Starting Y position
    int targetX;   // Target X position
    int targetY;   // Target Y position
    int speed;
} Ghost;


Ghost default_ghosts[] = {
    {150, 150, 150, 150, 450, 150, 10}, //y constant r-l (top left)
    {150, 200, 150, 200, 150, 400, 10}, //x constant u-d
    {700, 150, 700, 150, 500, 150, 10}, //y constant l-r (top right)
    {150, 650, 150, 650, 150, 450, 10}, //x constant d-u
    {700, 750, 700, 750, 500, 750, 10}, //y constant l-r (bottom right)
    {150, 750, 150, 750, 450, 750, 10}, //bottom left
    {700, 200, 150, 200, 700, 400, 10}, //not so random top parallelogram
    {700, 650, 150, 650, 700, 450, 10}, //random bottom parallelogram
    //{700, 650, 150, 650, 700, 450, 10},
    {700, 200, 700, 250, 700, 450, 10},
};

Ghost ghosts[] = {
    {150, 150, 150, 150, 450, 150, 10}, //y constant r-l (top left)
    {150, 200, 150, 200, 150, 400, 10}, //x constant u-d
    {700, 150, 700, 150, 500, 150, 10}, //y constant l-r (top right)
    {150, 650, 150, 650, 150, 450, 10}, //x constant d-u
    {700, 750, 700, 750, 500, 750, 10}, //y constant l-r (bottom right)
    {150, 750, 150, 750, 450, 750, 10}, //bottom left
    {700, 200, 150, 200, 700, 400, 10}, //not so random top parallelogram
    {700, 650, 150, 650, 700, 450, 10}, //random bottom parallelogram
    //{700, 650, 150, 650, 700, 450, 10},
    {700, 200, 700, 250, 700, 450, 10},
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
    int delta; // set to 0 to set count to 0
    bool isVisible;  // New flag for visibility
} Pumpkin;

Wall walls[] = {

    // Top border wall sections
    {0, 0}, {50, 0}, {100, 0}, {150, 0}, {200, 0}, {250, 0}, {300, 0}, {350, 0}, {400, 0}, {450, 0}, {500, 0}, {550, 0}, {600, 0}, {650, 0}, {700, 0}, {750, 0}, {800, 0}, {850, 0}, 

    // Left border wall sections
    {0, 50}, {0, 100}, {0, 150}, {0, 200}, {0, 250}, {0, 300}, {0, 350}, {0, 400}, {0, 450}, {0, 500}, {0, 550}, {0, 600}, {0, 650}, {0, 700}, {0, 750}, {0, 800}, {0, 850},

    // Right border wall sections
    {900, 50}, {900, 100}, {900, 150}, {900, 200}, {900, 250}, {900, 300}, {900, 350}, {900, 400}, {900, 450}, {900, 500}, {900, 550}, {900, 600}, {900, 650}, {900, 700}, {900, 750}, {900, 800}, {900, 850},

    // Bottom border wall sections
    {0, 900}, {100, 900}, {150, 900}, {200, 900}, {250, 900}, {300, 900}, {350, 900}, {400, 900}, {450, 900}, {500, 900}, {550, 900}, {600, 900}, {650, 900}, {700, 900}, {750, 900}, {800, 900}, {850, 900}, 
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
    {740, 290}, {740, 340}, {740, 390}, {740, 440}, {740, 490}, {740, 540}, {740, 590}, {790, 450},

    //LEFT
    {50, 450}, {100, 450}, {100, 350}, {100, 300}, {100, 400}, {100, 600}, {100, 550}, {100, 500},
    {150, 450}, {200, 450}, {250, 450},

    //DIAGONALS (TR-BL)
    {775,75},{825,25},{725,125},{675,175},{625,225},{575,275},{275,575},{225,625},{175, 675},{125,725},{75,775},{25,825}


};

Pumpkin pumpkins[] = {
    {150, 50, 1, true}, //1
    {350, 50, 1, true},//2
    {550, 50, 1, true},//3
    {750, 50, 1, true},//4
    {58, 150, 1, true},//5
    {58, 350, 1, true},//6
    {58, 550, 1, true},//7
    {58, 740, 1, true},//8
    {150, 800, 1, true},//9
    {350, 800, 1, true},//10
    {550, 800, 1, true},//11
    {800, 730, 1, true},//12
    {800, 150, 1, true},//13
    {800, 350, 1, true},//14
    {800, 550, 1, true},//15
    {715, 800, 1, true},//16
};

const size_t NUM_PUMPKINS = sizeof(pumpkins) / sizeof(Pumpkin);

const char prompts[][48] = {
    "\"From the sum of my past, my future will grow,",
    "In a sequence of numbers, I endlessly flow.\"",
    "\"Sixteen pumpkins, with candies alright,",
    "Squares alone hold tricks inside!\""
};

void setup_p1() {
    for (int i = 0; i < NUM_PUMPKINS; i++) 
        if (i+1 == 1 || i+1 == 2 || i+1 == 3 || i+1 == 5|| i+1 == 8 || i+1 == 11)
            pumpkins[i].delta = 1;
        else
            pumpkins[i].delta = 0;
}

void setup_p2() {
    for (int i = 0; i < NUM_PUMPKINS; i++) 
        if (i+1 == 1 || i+1 == 4 || i+1 == 9 || i+1 == 16)
            pumpkins[i].delta = 0;
        else
            pumpkins[i].delta = 1;
}

void draw_prompt(int prompt, int w, int h) {
    int idx = (prompt-1) * 2;
    const char *fl = prompts[idx];
    const char *sl = prompts[idx+1];
    int fll = MeasureText(fl, 25);
    int sll = MeasureText(sl, 25);
    DrawRectangle((w-fll)/2-2, 1, fll+5, 25, (Color){0, 0, 0, CANDY_ALPHA});
    DrawText(fl, (w-fll)/2, 1, 25, WHITE);
    DrawRectangle((w-sll)/2-2, 26, sll+5, 25, (Color){0, 0, 0, CANDY_ALPHA});
    DrawText(sl, (w-sll)/2, 26, 25, WHITE);
}

bool are_all_good_pumpkins_gone() {
    bool res = true;
    for (int i = 0; i < NUM_PUMPKINS; i++) {
        if (pumpkins[i].delta > 0 && pumpkins[i].isVisible) res = false;
    }
    return res;
}
 
bool has_lost_game = false;
int place = -1;

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
          memcpy(ghosts, default_ghosts, sizeof(default_ghosts));
          add_client(nid, x, y);
        } else if (strncmp(buffer, "leave_", 5) == 0) {
          int nid, x, y;
          sscanf(buffer, "leave_%d", &nid);
          remove_client(nid);
        } else if (strncmp(buffer, "pos_", 4) == 0) {
          int nid, x, y;
          sscanf(buffer, "pos_%d-(%d,%d)", &nid, &x, &y);
          update_client(nid, x, y);
        } else if (strncmp(buffer, "lose", 4) == 0) {
          has_lost_game = true;
        } else if (strncmp(buffer, "pumpkin-", 8) == 0) {
          int idx;
          sscanf(buffer, "pumpkin-(%d)", &idx);
          if (idx < NUM_PUMPKINS && idx >= 0) pumpkins[idx].isVisible = false;
          if (are_all_good_pumpkins_gone()) send_data(sockfd, "end\n", 3);
        } else if (strncmp(buffer, "place-", 6) == 0) {
          sscanf(buffer, "place-(%d)", &place);
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

// Function to check collision between player and walls
bool CheckCollisionWithWalls(int x, int y) {
    Rectangle playerRect = {x, y, 25, 25};  // Player's bounding box
    for (int i = 0; i < sizeof(walls) / sizeof(Wall); i++) {
        Rectangle wallRect = {walls[i].x-7, walls[i].y-10, WALL_WIDTH, WALL_HEIGHT};
        if (CheckCollisionRecs(playerRect, wallRect)) {
            return true;  // Collision detected
        }
    }
    return false;  // No collision
}

// Function to check collision between player and pumpkins
void CheckCollisionWithPumpkins(int x, int y, int *score) {
    Rectangle playerRect = {x, y, 25, 25};  // Player's bounding box
    for (int i = 0; i < NUM_PUMPKINS; i++) {
        if (pumpkins[i].isVisible) {  // Only check visible pumpkins
            Rectangle pumpkinRect = {pumpkins[i].x, pumpkins[i].y, WALL_WIDTH, WALL_HEIGHT};
            if (CheckCollisionRecs(playerRect, pumpkinRect)) {
                pumpkins[i].isVisible = false;  // Hide the pumpkin
                int d = pumpkins[i].delta;
                if (d == 0) *score = 0;
                else *score += d;
                send_pumpkin(sockfd, i);
                send_candies(sockfd, *score);
                if (are_all_good_pumpkins_gone()) send_data(sockfd, "end\n", 3);
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
// Function to move the ghosts between predefined coordinates
// Function to move the ghosts between predefined coordinates
void MoveGhosts(Ghost ghosts[], int numGhosts) {
    for (int i = 0; i < numGhosts; i++) {
        // Move along x-axis
        if (ghosts[i].x < ghosts[i].targetX) {
            ghosts[i].x += ghosts[i].speed;
            if (ghosts[i].x > ghosts[i].targetX) ghosts[i].x = ghosts[i].targetX;  // Clamp position
        } else if (ghosts[i].x > ghosts[i].targetX) {
            ghosts[i].x -= ghosts[i].speed;
            if (ghosts[i].x < ghosts[i].targetX) ghosts[i].x = ghosts[i].targetX;  // Clamp position
        }

        // Move along y-axis
        if (ghosts[i].y < ghosts[i].targetY) {
            ghosts[i].y += ghosts[i].speed;
            if (ghosts[i].y > ghosts[i].targetY) ghosts[i].y = ghosts[i].targetY;  // Clamp position
        } else if (ghosts[i].y > ghosts[i].targetY) {
            ghosts[i].y -= ghosts[i].speed;
            if (ghosts[i].y < ghosts[i].targetY) ghosts[i].y = ghosts[i].targetY;  // Clamp position
        }

        // Check if the ghost has reached its target position
        if (abs(ghosts[i].x - ghosts[i].targetX) < ghosts[i].speed &&
            abs(ghosts[i].y - ghosts[i].targetY) < ghosts[i].speed) {
            // Swap the target and starting positions
            int tempX = ghosts[i].targetX;
            int tempY = ghosts[i].targetY;
            ghosts[i].targetX = ghosts[i].startX;
            ghosts[i].targetY = ghosts[i].startY;
            ghosts[i].startX = tempX;
            ghosts[i].startY = tempY;

            // Debug statement to confirm the swap
            // TraceLog(LOG_INFO, "Ghost at (%d, %d) swapped to (%d, %d)", ghosts[i].x, ghosts[i].y, ghosts[i].targetX, ghosts[i].targetY);
        }
    }
}

const char title[] = "Crazy Candy Chaos";
const char st[] = "Press any key to start";
const char t[] = "Game over!";
const char st1[] = "You lost all your candies.";
const char sm[] = "Press any key to exit";
const char c[] = "Connecting...";
const char inst1[] = "Instructions:";
const char inst2[] = "The riddle on top shows the way,";
const char inst3[] = "Pick the right pumpkins, don't go astray,";
const char inst4[] = "For the right ones contain candy you crave,";
const char inst5[] = "And the wrong ones shall take it all away.";

int main(void) {

    InitWindow(900, 900, "YAAAS");
    int w = GetScreenWidth(), h = GetScreenHeight();
    while (GetKeyPressed() == 0) {
        BeginDrawing();
            ClearBackground(BLUE);  // Set a background color for the start screen
            DrawText(title, (w-MeasureText(title, 50))/2, h/2-200, 50, WHITE);
            DrawText(st, (w-MeasureText(st, 30))/2, h/2-100, 30, WHITE);
            DrawText(inst1, 50, h/2+100, 20, WHITE);
            DrawText(inst2, 50, h/2+150, 20, WHITE);
            DrawText(inst3, 50, h/2+180, 20, WHITE);
            DrawText(inst4, 50, h/2+210, 20, WHITE);
            DrawText(inst5, 50, h/2+240, 20, WHITE);
        EndDrawing();
    }

  BeginDrawing();
    ClearBackground(GRAY);
    DrawText(c, (w-MeasureText(c, 30))/2, h/2, 30, WHITE);
  EndDrawing();
    
    Rectangle sourceRec = { 0, 0, 50, 50 };
    Texture2D sprites[] = {
        LoadTexture("./Finals/Gangster_Final.png"),
        LoadTexture("./Finals/Girl_Final.png"),
        LoadTexture("./Finals/Trader_Final.png")
    };
    Texture2D wallTexture = LoadTexture("./wall/sprite.png");
    Texture2D pumpkinTexture = LoadTexture("./wall/pumpkin.png");
    Texture2D candyTexture = LoadTexture("./wall/candy.png");
    Texture2D ghostTexture = LoadTexture("./wall/ghost.png");

  int id, x, y, candies, prompt;

  TraceLog(LOG_INFO, "Connecting to server");
  connect_to_server(HOST, &servinfo, &sockfd, s);
  TraceLog(LOG_INFO, "Getting start pos");
  fetch_and_set_starting_pos(sockfd, &id, &x, &y);
  fetch_and_set_starting_candies(sockfd, &candies);
  fetch_and_set_starting_prompt(sockfd, &prompt);
  TraceLog(LOG_INFO, "Getting other connected clients");
  get_clients(sockfd, add_client);

  if (prompt == 1) setup_p1();
  else if (prompt == 2) setup_p2();

  SetTargetFPS(30);

    while (!WindowShouldClose()) {
        if (has_lost_game) {
            close(sockfd);
            while (GetKeyPressed() == 0) {
                BeginDrawing();
                    ClearBackground(BLACK);
                    DrawText(t, (w-MeasureText(t, 50))/2, h/2-100, 50, RED);
                    DrawText(st1, (w-MeasureText(st1, 30))/2, h/2, 30, RED);
                    DrawText(sm, (w-MeasureText(st1, 20))/2, 800, 20, RED);
                EndDrawing();
            }
            goto end;
        }
        if (place != -1) {
            close(sockfd);
            char x[20];
            if (place <= 3) snprintf(x, 20, "%s Place!", place == 1 ? "1st" : place == 2 ? "2nd" : "3rd");
            else snprintf(x, 20, "%dth Place!", place);
            char can[32];
            snprintf(can, 32, candies == 1 ? "You collected %d candy." : "You collected %d candies.", candies);
            while (GetKeyPressed() == 0) {
                BeginDrawing();
                    ClearBackground(BLACK);
                    DrawText(x, (w-MeasureText(x, 50))/2, h/2-100, 50, YELLOW);
                    DrawText(can, (w-MeasureText(can, 30))/2, h/2, 30, YELLOW);
                    DrawText(sm, (w-MeasureText(st1, 20))/2, 800, 20, RED);
                EndDrawing();
            }
            goto end;
        }
        listen_for_data(sockfd);
        int prevX = x;
        int prevY = y;

        // Update player position with boundary checks
        if ((IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) && y > wallTexture.height) y -= 10;
        if ((IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) && x > wallTexture.width) x -= 10;
        if ((IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) && y < GetScreenHeight() - wallTexture.height - sourceRec.height) y += 10;
        if ((IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) && x < GetScreenWidth() - wallTexture.width - sourceRec.width) x += 10;

        if (CheckCollisionWithWalls(x, y)) {
            x = prevX;
            y = prevY;
        }

        // Check collisions with pumpkins
        CheckCollisionWithPumpkins(x, y, &candies);
        MoveGhosts(ghosts, NUM_GHOSTS);


        // Check if player collides with the ghost
        if (CheckCollisionWithAnyGhost(x, y, ghosts, NUM_GHOSTS)) {
            x = 450;
            y = 450;
            candies--;
            send_candies(sockfd, candies);
        }

        if (x != prevX || y != prevY) send_pos(sockfd, x, y);

        BeginDrawing();
            ClearBackground(GRAY);
            
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

            for (int i = 0; i < NUM_PUMPKINS; i++) {
                if (pumpkins[i].isVisible) {  
                    int newWidth = pumpkinTexture.width * 2; 
                    int newHeight = pumpkinTexture.height * 2; 
                    // Draw the pumpkin with the new size
                    DrawTexturePro(pumpkinTexture, 
                        (Rectangle){0, 0, pumpkinTexture.width, pumpkinTexture.height}, 
                        (Rectangle){pumpkins[i].x, pumpkins[i].y, newWidth, newHeight}, 
                        (Vector2){0, 0}, 
                        0, WHITE);
                    char buf[3];
                    snprintf(buf, 3, "%.2d", i+1);
                    DrawText(buf, pumpkins[i].x - 5, pumpkins[i].y, 20, WHITE);
                }
            }

            for (int i = 0; i < NUM_GHOSTS; i++) {
                DrawTexture(ghostTexture, ghosts[i].x, ghosts[i].y, WHITE);
            }

            // Draw player sprite
            DrawTextureRec(sprites[id%3], sourceRec, (Vector2){ x, y }, WHITE);

          draw_prompt(prompt, w, h);

            // Draw candy score
          draw_candies(candyTexture, candies);

            // Draw other players
          draw_clients(sprites, sourceRec);
            //Draw ghost
            
        EndDrawing();
    }

    close(sockfd);
end:
    UnloadTexture(sprites[0]);
    UnloadTexture(sprites[1]);
    UnloadTexture(sprites[2]);
    UnloadTexture(wallTexture);
    UnloadTexture(pumpkinTexture);  // Unload pumpkin texture
    CloseWindow();

  free_clients(root.next);
  return 0;
}
