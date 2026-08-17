#include "raylib.h"

#include "assets.h"

#if defined(PLATFORM_WEB)
#include <emscripten/emscripten.h>
#endif

#define SCREEN_WIDTH  800
#define SCREEN_HEIGHT 600

static void UpdateDrawFrame(void);

int main(void)
{
    InitAssets();
    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "rlsnake");

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose()) UpdateDrawFrame();
#endif

    CloseWindow();
    return 0;
}

static void UpdateDrawFrame(void)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);
    EndDrawing();
}
