#include <cmath>
#include <iostream>

#include "World.h"
#include "raylib.h"

int main() {
  std::cout << "Hello, World!" << std::endl;

  InitWindow(800, 600, "Physics");
  SetTargetFPS(FPS);
  World world;
  Box wall(Vector2 { 600, 0 }, Vector2 { 10, 2000 }, 1, GREEN);
  Box box(Vector2 { 200, 200 }, Vector2 { 25, 25 }, 1, WHITE);
  box.velocity = Vector2 { 2000, -500 };
  world.SetMode(Discrete60);
  world.SpawnFast(box);
  world.SpawnStatic(wall);

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
