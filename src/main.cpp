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

  float tAccum = 0.;
  float fixedDt = 1. / static_cast<float>(world.regularSimRate);

  while (!WindowShouldClose()) {
    tAccum += GetFrameTime();

    while (tAccum >= fixedDt) {
      world.Simulate(fixedDt);
      tAccum -= fixedDt;
    }
    float alpha = tAccum / fixedDt; // how much longer till next step?

    BeginDrawing();

    ClearBackground(BLACK);
    world.DrawBoxes(alpha);
    world.DrawStats();

    EndDrawing();
  }
  return 0;
}
