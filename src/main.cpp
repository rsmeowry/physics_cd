#include <cmath>
#include <iostream>

#include "World.h"
#include "raylib.h"

void BuildCCDScene(World& world);
void BuildMRScene(World& world);

int main() {
  std::cout << "Hello, World!" << std::endl;

  InitWindow(800, 600, "Physics");
  SetTargetFPS(FPS);
  World world;

  float tAccum = 0.;
  float fixedDt = 1. / static_cast<float>(world.regularSimRate);

  while (!WindowShouldClose()) {
    tAccum += GetFrameTime();

    while (tAccum >= fixedDt) {
      world.Simulate(fixedDt);
      tAccum -= fixedDt;
    }
    float alpha = tAccum / fixedDt; // how much longer till next step?

    if (IsKeyPressed(KEY_ONE)) {
      srand(1);
      world.Clear();
      BuildMRScene(world);
    }
    if (IsKeyPressed(KEY_TWO)) {
      srand(1);
      world.Clear();
      BuildCCDScene(world);
    }
    if (IsKeyPressed(KEY_THREE)) {
      srand(1);
      world.Clear();
      BuildMRScene(world);
      world.SetMode(Discrete60);
    }
    if (IsKeyPressed(KEY_FOUR)) {
      srand(1);
      world.Clear();
      BuildCCDScene(world);
      world.SetMode(Discrete60);
    }

    BeginDrawing();

    ClearBackground(BLACK);
    world.DrawBoxes(alpha);
    world.DrawStats();

    EndDrawing();
  }
  return 0;
}

void BuildCCDScene(World& world) {
  Box wall(Vector2 { 600, -200 }, Vector2 { 10, 2500 }, 1, GREEN);

  // 10 fast boxes
  for (int i = 0; i < 10; i++) {
    float r = (rand() % 100) / 100.;
    Box box(Vector2 { 200, 300 }, Vector2 { 15, 15 }, 1, WHITE);
    r = (rand() % 100) / 100.;
    float r2 = (rand() % 100) / 100.;
    box.velocity = Vector2 { 1500 + r * 1200, -200 + r2 * (-500) };
    world.SpawnFast(box);
  }

  world.SetMode(CCD60);
  world.SpawnStatic(wall);
}

void BuildMRScene(World& world) {

  // 5 fast boxes
  for (int i = 0; i < 5; i++) {
    float r = (rand() % 100) / 100.;
    Box box(Vector2 { 200, 200 }, Vector2 { 15, 15 }, 1, WHITE);
    r = (rand() % 100) / 100.;
    float r2 = (rand() % 100) / 100.;
    box.velocity = Vector2 { 1300 + r * 1000, -200 + r2 * (-500) };
    world.SpawnFast(box);
  }

  // 20 slow boxes
  for (int i = 0; i < 20; i++) {
    float r = (rand() % 100) / 100.;
    Box box(Vector2 { 200, 400 }, Vector2 { 15, 15 }, 1.5, GRAY);
    r = (rand() % 100) / 100.;
    float r2 = (rand() % 100) / 100.;
    box.velocity = Vector2 { 100 + r * 400, -200 + r2 * (-500) };
    world.SpawnSlow(box);

  }
  world.SetMode(Multi240);

  Box rwall(Vector2 { 600, -200 }, Vector2 { 30, 2500 }, 1, GREEN);
  Box floor(Vector2 { 0, 500 }, Vector2 { 1000, 30 }, 1, GREEN);
  Box lwall(Vector2 { 100, -200 }, Vector2 { 30, 2500 }, 1, GREEN);
  world.SpawnStatic(rwall);
  world.SpawnStatic(lwall);
  world.SpawnStatic(floor);

}