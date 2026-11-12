#include "World.h"

#include <algorithm>

#include "raymath.h"

void World::SimulateSlow(float dt) {
  for (auto& box : slow) {
    box.velocity = Vector2Add(box.velocity, Vector2Scale(gravity, dt));
    box.position = Vector2Add(box.position, Vector2Scale(box.velocity, dt));
    box.lifetime++;
    CollideStatics(box);
  }
  std::erase_if(slow, [this](const Box& b) {
    return b.lifetime > SLOW_BOX_LIFETIME_SECS * regularSimRate;
  });
  regularStepsPerFrame++;
}
void World::SimulateFast(float dt) {
  for (auto& box : fast) {
    bool wasLeftOfTheWall = (box.position.x + box.extent.x / 2) < (statics[0].position.x + statics[0].extent.x);

    box.velocity = Vector2Add(box.velocity, Vector2Scale(gravity, dt));
    box.position = Vector2Add(box.position, Vector2Scale(box.velocity, dt));
    box.lifetime++;
    CollideStatics(box);

    // checking whether we tunnelled
    if (wasLeftOfTheWall && (box.position.x + box.extent.x / 2) > (statics[0].position.x + statics[0].extent.x)) {
      box.color = RED;
      tunnelCount += 1;
    }
  }
  std::erase_if(fast, [this](const Box& b) {
    return b.lifetime > FAST_BOX_LIFETIME_SECS   * regularSimRate;
  });
  fastStepsPerFrame++;
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
  for (auto& b : statics)
    DrawRectangleV(b.position, b.extent, b.color);
  for (auto& b : slow)
    DrawRectangleV(Vector2Lerp(b.prevPosition, b.position, alpha), b.extent, b.color);
  for (auto& b : fast)
    DrawRectangleV(Vector2Lerp(b.prevPosition, b.position, alpha), b.extent, b.color);
}

void World::DrawStats() {
  statFrames++;
  statTime += GetFrameTime();
  if (statTime >= 1.0f) {
    regAvg = static_cast<float>(regularStepsPerFrame) / statFrames;
    fastAvg = static_cast<float>(fastStepsPerFrame) / statFrames;
    regularStepsPerFrame = fastStepsPerFrame = statFrames = 0;
    statTime = 0;
  }

  constexpr int offset = 25;
  DrawText(std::format("reg spf: {:.2f}", regAvg).c_str(), 25, 50, 15, RAYWHITE);
  DrawText(std::format("fst spf: {:.2f}", fastAvg).c_str(), 25, 50 + offset, 15, RAYWHITE);
  DrawText(std::format("tunnels: {}", tunnelCount).c_str(), 25, 50 + offset * 2, 15, RAYWHITE);
  DrawText(std::format("ccd hit: {}", ccdHits).c_str(), 25, 50 + offset * 3, 15, RAYWHITE);
}

void World::CollideStatics(Box &box) {
  for (auto& s : statics) {
    Rectangle o = GetCollisionRec(box.AsRect(), s.AsRect());
    if (o.width <= 0 || o.height <= 0) continue;

    // determining normal
    if (o.width < o.height) {
      float dir = (box.position.x < s.position.x) ? -1.0f : 1.0f;
      box.position.x += dir * o.width;
      if (box.velocity.x * dir < 0)
        box.velocity.x *= -restitution;
    } else {
      float dir = (box.position.y < s.position.y) ? -1.0f : 1.0f;
      box.position.y += dir * o.height;
      if (box.velocity.y * dir < 0)
        box.velocity.y *= -restitution;
    }
  }
}
