#include "World.h"

#include <algorithm>

#include "raymath.h"

void World::SimulateSlow(float dt) {
  for (auto& box : slow) {
    box.velocity = Vector2Add(box.velocity, Vector2Scale(gravity, dt));
    box.position = Vector2Add(box.position, Vector2Scale(box.velocity, dt));
    box.lifetime++;
  }
  std::erase_if(slow, [this](const Box& b) {
    return b.lifetime > SLOW_BOX_LIFETIME_SECS * regularSimRate;
  });
  regularStepsPerFrame++;
}
void World::SimulateFast(float dt) {
  for (auto& box : fast) {
    box.velocity = Vector2Add(box.velocity, Vector2Scale(gravity, dt));
    box.position = Vector2Add(box.position, Vector2Scale(box.velocity, dt));
    box.lifetime++;
  }
  std::erase_if(fast, [this](const Box& b) {
    return b.lifetime > SLOW_BOX_LIFETIME_SECS * regularSimRate;
  });
  regularStepsPerFrame++;
}

void World::Simulate(float dt) {
  for (auto& b : slow)
    b.prevPosition = b.position;
  for (auto& b : fast)
    b.prevPosition = b.position;

  SimulateSlow(dt);

  auto fastSub = fastSimRate / regularSimRate;
  for (int i = 0; i < fastSub; ++i)
    SimulateFast(dt / static_cast<float>(fastSub));
}

void World::DrawBoxes(float alpha) {
  for (auto box: this->statics) {
    DrawRectangle(box.position.x, box.position.y, box.extent.x, box.extent.y, box.color);
  }
  for (auto box: this->slow) {
    DrawRectangle(box.position.x, box.position.y, box.extent.x , box.extent.y, box.color);
  }
  for (auto box: this->fast) {
    DrawRectangle(box.position.x, box.position.y, box.extent.x, box.extent.y, box.color);
  }

}

void World::DrawStats() {
  constexpr int offset = 25;
  DrawText(std::format("reg spf: {0}", regularStepsPerFrame).c_str(), 25, 50, 15, RAYWHITE);
  DrawText(std::format("fst spf: {0}", fastStepsPerFrame).c_str(), 25, 50 + offset, 15, RAYWHITE);
  DrawText(std::format("tunnels: {0}", tunnelCount).c_str(), 25, 50 + offset * 2, 15, RAYWHITE);
  DrawText(std::format("ccd hit: {0}", ccdHits).c_str(), 25, 50 + offset * 3, 15, RAYWHITE);

  regularStepsPerFrame = 0;
  fastStepsPerFrame = 0;
}
