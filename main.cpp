#include <cmath>
#include <iostream>

#include "raylib.h"

int main() {
  std::cout << "Hello, World!" << std::endl;

  InitWindow(800, 600, "Physics");
  SetTargetFPS(144);

  while (!WindowShouldClose()) {
    BeginDrawing();

    ClearBackground(RAYWHITE);
    DrawText("hello , wworld", 190, 200 + 200 * sin(GetTime()), 20, LIGHTGRAY);

    EndDrawing();
  }
  return 0;
}
