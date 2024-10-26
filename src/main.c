#include "raylib.h"
#include <netdb.h>
#include <netinet/in.h>

int main(void)
{
    InitWindow(912, 912, "zao shang hao");

    int x = 100;
    int y = 100;

    TraceLog(LOG_INFO, "Connecting to server");

    SetTargetFPS(30);

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground((Color){0, 0, 0, 0});
            DrawRectangle(x, y, 50, 50, RED);
            if (IsKeyDown(KEY_W)) y -= 10;
            if (IsKeyDown(KEY_A)) x -= 10;
            if (IsKeyDown(KEY_S)) y += 10;
            if (IsKeyDown(KEY_D)) x += 10;
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
