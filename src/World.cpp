#include "World.h"

#include <algorithm>

#include "raymath.h"

void World::SimulateSlow(float dt) {
  for (auto& box : slow) {
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
  std::erase_if(slow, [this](const Box& b) {
    return b.lifetime > SLOW_BOX_LIFETIME_SECS * regularSimRate;
  });
  regularStepsPerFrame++;
}

// https://github.com/srumut/raylib_Swept_AABB/blob/main/main.go

bool DoSlab(float p, float dv, float lo, float hi, Vector2 axis, float& tEnter, float& tExit, Vector2& normal) {
  if (std::fabs(dv) < 1e-8f)
    return p > lo && p < hi; // miss
  float t1 = (lo - p) / dv, t2 = (hi - p) / dv;
  float tin = std::min(t1, t2);
  float tout = std::max(t1, t2);
  if (tin > tEnter) {
    tEnter = tin;
    normal = dv > 0 ? Vector2Negate(axis) : axis;
  }
  tExit = std::min(tExit, tout);
  return true;
}

bool World::SweepAABB(Box &a, Vector2 dir, Box &b, float &tHit, Vector2 &normal) {
  float tEnter = -INFINITY;
  float tExit = INFINITY;
  normal = {0, 0};

  if (!DoSlab(a.position.x, dir.x, b.position.x - a.extent.x, b.position.x + b.extent.x, {1, 0}, tEnter, tExit, normal))
    return false;
  if (!DoSlab(a.position.y, dir.y, b.position.y - a.extent.y, b.position.y + b.extent.y, {0, 1}, tEnter, tExit, normal))
    return false;
  if (tEnter > tExit || tEnter < 0.f || tEnter > 1.f)
    return false;

  tHit = tEnter;

  return true;
}

void World::SimulateCCD(Box &box, float dt) {
  float remaining = dt;
  for (int iter = 0; iter < 4 && remaining > 0; ++iter) {
    Vector2 d = Vector2Scale(box.velocity, remaining);

    float tMin = 1.f;
    Vector2 nMin {};
    bool hit = false;
    for (auto& s : statics) { // getting closest hit among all statics
      float t;
      Vector2 normal;
      if (SweepAABB(box, d, s, t, normal) && t < tMin) {
        tMin = t;
        nMin = normal;
        hit = true;
      }
    }

    box.position = Vector2Add(box.position, Vector2Scale(d, tMin)); // moving towards the point
    if (!hit)
      break;

    box.position = Vector2Add(box.position, Vector2Scale(nMin, 1e-3f)); // offset
    float vn = Vector2DotProduct(box.velocity, nMin); // reaction, velocity * normal
    if (vn < 0)
      box.velocity = Vector2Subtract(box.velocity, Vector2Scale(nMin, (1 + restitution) * vn));

    remaining *= (1.f - tMin); // remainder of the step
    ccdHits++;
  }
}

void World::SimulateFast(float dt) {
  for (auto& box : fast) {
    bool wasLeftOfTheWall = (box.position.x + box.extent.x / 2) < (statics[0].position.x + statics[0].extent.x);

    box.velocity = Vector2Add(box.velocity, Vector2Scale(gravity, dt));
    if (this->simulationMode == CCD60) {
      SimulateCCD(box, dt);
    } else {
      box.position = Vector2Add(box.position, Vector2Scale(box.velocity, dt));
      CollideStatics(box);
    }
    box.lifetime++;

    // checking whether we tunnelled
    if (wasLeftOfTheWall && (box.position.x + box.extent.x / 2) > (statics[0].position.x + statics[0].extent.x)) {
      box.color = RED;
      tunnelCount += 1;
    }
  }
  std::erase_if(fast, [this](const Box& b) {
    return b.lifetime > FAST_BOX_LIFETIME_SECS * fastSimRate;
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
