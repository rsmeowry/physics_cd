#include <cmath>
#include <iostream>

#include "World.h"
#include "raylib.h"

int main() {
  std::cout << "Hello, World!" << std::endl;

  InitWindow(800, 600, "Physics");
  SetTargetFPS(FPS);
  World world;
  Box box(Vector2 { 200, 200 }, Vector2 { 50, 50 }, 1, RED);;
  world.SpawnFast(box);

  auto slowSimsPerFrame = static_cast<int>(static_cast<float>(world.regularSimRate) / static_cast<float>(FPS));
  auto fastSimsPerFrame = static_cast<int>(static_cast<float>(world.fastSimRate) / static_cast<float>(FPS));

  std::cout << " " << fastSimsPerFrame << " " << slowSimsPerFrame << std::endl;

  while (!WindowShouldClose()) {
    // simulating slow
    for (int i = 0; i < slowSimsPerFrame; i++) {
      world.SimulateSlow();
    }

    // simulating fast
    for (int i = 0; i < fastSimsPerFrame; i++) {
      world.SimulateFast();
    }

    BeginDrawing();

    ClearBackground(BLACK);
    world.DrawBoxes();
    world.DrawStats();

    EndDrawing();
  }
  return 0;
}
