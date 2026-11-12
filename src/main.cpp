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

  // 10 fast boxes
  for (int i = 0; i < 10; i++) {
    float r = (rand() % 100) / 100.;
    Box box(Vector2 { 200, 200 }, Vector2 { 15, 15 }, 1, WHITE);
    r = (rand() % 100) / 100.;
    float r2 = (rand() % 100) / 100.;
    box.velocity = Vector2 { 1200 + r * 1200, -200 + r * (-500) };
    world.SpawnFast(box);
  }
  world.SetMode(Discrete60);
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
