#include "raylib.h"
#include <netdb.h>
#include <netinet/in.h>

int main(void)
{
    InitWindow(912, 912, "YAAAS");
    Rectangle sourceRec = { 0, 0, 50, 50 }; // Line 8

    Texture2D sprite = LoadTexture("./Finals/Girl_Final.png"); // Line 7
    Texture2D wallTexture = LoadTexture("./wall/sprite.png"); // Line 9: Load the wall texture

    int x = 100;
    int y = 100;

    const int borderThickness = wallTexture.height; // Line 10: Set border thickness based on wall height
    const int playableWidth = GetScreenWidth() - borderThickness * 2; // Width inside the border
    const int playableHeight = GetScreenHeight() - borderThickness * 2; // Height inside the border
    
    TraceLog(LOG_INFO, "Connecting to server");

    SetTargetFPS(30);

    while (!WindowShouldClose()) {

        if (IsKeyDown(KEY_W) && y > borderThickness) y -= 10; // Prevent moving beyond top border
        if (IsKeyDown(KEY_A) && x > borderThickness) x -= 10; // Prevent moving beyond left border
        if (IsKeyDown(KEY_S) && y < GetScreenHeight() - borderThickness - sourceRec.height) y += 10; // Prevent moving beyond bottom border
        if (IsKeyDown(KEY_D) && x < GetScreenWidth() - borderThickness - sourceRec.width) x += 10; // Prevent moving beyond right border
        
        BeginDrawing();
            ClearBackground((Color){0, 0, 0, 0});
            
            // Draw the wall texture as the border
            for (int i = 0; i < GetScreenWidth() / wallTexture.width; i++) {
                // Top wall
                DrawTexture(wallTexture, i * wallTexture.width, 0, WHITE); 
                // Bottom wall
                DrawTexture(wallTexture, i * wallTexture.width, GetScreenHeight() - wallTexture.height, WHITE);
            }

            for (int i = 0; i < GetScreenHeight() / wallTexture.height; i++) {
                // Left wall
                DrawTexture(wallTexture, 0, i * wallTexture.height, WHITE);
                // Right wall
                DrawTexture(wallTexture, GetScreenWidth() - wallTexture.width, i * wallTexture.height, WHITE);
            }
        
            DrawTextureRec(sprite, sourceRec, (Vector2){ x, y }, WHITE);

        EndDrawing();
    }

    UnloadTexture(sprite);
    UnloadTexture(wallTexture); // Line 20: Unload the wall texture
    CloseWindow();

    return 0;
}
