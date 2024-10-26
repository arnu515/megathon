#include "raylib.h"

int main(void)
{
    InitWindow(912, 912, "zao shang hao");

    while (!WindowShouldClose()) {
        BeginDrawing();
            ClearBackground((Color){0, 0, 0, 0});
            DrawText("rent free", 100, 100, 40, RAYWHITE);
        EndDrawing();
    }

    CloseWindow();

    return 0;
}
