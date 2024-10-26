#include "raylib.h"
#include <netdb.h>
#include <netinet/in.h>

#define WALL_WIDTH 50
#define WALL_HEIGHT 50

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
    {100, 50, true},  // Set initial visibility to true
    {400, 50, true},
    {500, 50, true},
    {750, 50, true},
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

int main(void) {
    InitWindow(900, 900, "YAAAS");
    
    Rectangle sourceRec = { 0, 0, 50, 50 };
    Texture2D sprite = LoadTexture("./Finals/Girl_Final.png");
    Texture2D wallTexture = LoadTexture("./wall/sprite.png");
    Texture2D pumpkinTexture = LoadTexture("./Finals/Yogi_Final.png");

    int x = 456;
    int y = 456;
    int score = 0;  // Initialize score

    SetTargetFPS(30);

    while (!WindowShouldClose()) {
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
        CheckCollisionWithPumpkins(x, y, &score);

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

            // Draw player sprite
            DrawTextureRec(sprite, sourceRec, (Vector2){ x, y }, WHITE);

            // Display the score
            DrawText(TextFormat("Score: %d", score), 10, 10, 20, WHITE);

        EndDrawing();
    }

    UnloadTexture(sprite);
    UnloadTexture(wallTexture);
    UnloadTexture(pumpkinTexture);  // Unload pumpkin texture
    CloseWindow();

    return 0;
}
