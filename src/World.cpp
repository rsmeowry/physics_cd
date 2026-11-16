#include "World.h"

#include <algorithm>
#include <chrono>

#include "raymath.h"

#define SLOW_RESOLUTIONS 10

void World::SimulateSlow(float dt) {
  for (auto& box : slow) {
    bool wasLeftOfTheWall = (box.position.x + box.extent.x / 2) < (statics[0].position.x + statics[0].extent.x);

    box.velocity = Vector2Add(box.velocity, Vector2Scale(gravity, dt));
    box.position = Vector2Add(box.position, Vector2Scale(box.velocity, dt));
    box.lifetime++;

    // checking whether we tunnelled
    if (wasLeftOfTheWall && (box.position.x + box.extent.x / 2) > (statics[0].position.x + statics[0].extent.x)) {
      box.color = RED;
      tunnelCount += 1;
    }
  }

  for (auto i = 0; i < SLOW_RESOLUTIONS; i++) {
    ResolveSlow();
  }

  std::erase_if(slow, [this](const Box& b) {
    return b.lifetime > SLOW_BOX_LIFETIME_SECS * regularSimRate;
  });
  regularStepsPerFrame++;
}

void World::ResolveSlow() {
  for (auto i = 0; i < slow.size(); ++i)
    for (auto j = i + 1; j < slow.size(); ++j)
      CollidePair(slow[i], slow[j], true);

  for (auto& b : slow)
    for (auto& s : statics)
      CollidePair(b, s, false);
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

void World::SimulateFast(float dt, float t) {
  if (statics.empty())
    return; // empty scene

  float wallRight = statics[0].position.x + statics[0].extent.x;
  std::vector<char> wasLeft(fast.size());

  for (auto i = 0; i < fast.size(); ++i) {
    auto &box = fast[i];
    wasLeft[i] = box.position.x + box.extent.x / 2 < wallRight;

    box.velocity = Vector2Add(box.velocity, Vector2Scale(gravity, dt));
    if (this->simulationMode == CCD60) {
      SimulateCCD(box, dt);
    } else {
      box.position = Vector2Add(box.position, Vector2Scale(box.velocity, dt));
    }
    box.lifetime++;

  }

  ResolveFast(t);

  // checking whether we tunnelled
  for (size_t i = 0; i < fast.size(); ++i) {
    if (wasLeft[i] && fast[i].position.x + fast[i].extent.x / 2 > wallRight) {
      fast[i].color = RED;
      tunnelCount++;
    }
  }

  std::erase_if(fast, [this](const Box& b) {
    return b.lifetime > FAST_BOX_LIFETIME_SECS * fastSimRate;
  });

  fastStepsPerFrame++;
}

void World::ResolveFast(float t) {
  for (auto i = 0; i < fast.size(); ++i)
    for (auto j = i + 1; j < fast.size(); ++j)
      CollidePair(fast[i], fast[j], true);

  // slow is in the future, move it back in time
  for (auto& s : slow) {
    Box k = s;
    k.position = Vector2Lerp(s.prevPosition, s.position, t);
    for (auto& f : fast)
      CollidePair(f, k, false); // push only fast
    s.velocity = k.velocity; // delay impulse
  }

  for (auto& f : fast)
    for (auto& st : statics)
      CollidePair(f, st, false);
}

void World::Simulate(float dt) {
  auto t0 = std::chrono::steady_clock::now();
  for (auto& b : slow)
    b.prevPosition = b.position;
  for (auto& b : fast)
    b.prevPosition = b.position;

  SimulateSlow(dt);

  auto fastSub = fastSimRate / regularSimRate;
  for (int i = 0; i < fastSub; ++i)
    SimulateFast(dt / static_cast<float>(fastSub), static_cast<float>(i + 1) / static_cast<float>(fastSub));

  physMs += std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
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
    double f = physMs / statFrames;
    double s = regularStepsPerFrame ? physMs / regularStepsPerFrame : 0;
    constexpr double k = 0.2;
    msPerFrame = msPerFrame == 0 ? f : msPerFrame * (1 - k) + f * k;
    msPerStep = msPerStep  == 0 ? s : msPerStep  * (1 - k) + s * k;

    regAvg = static_cast<float>(regularStepsPerFrame) / statFrames;
    fastAvg = static_cast<float>(fastStepsPerFrame) / statFrames;
    regularStepsPerFrame = 0;
    fastStepsPerFrame = 0;
    statFrames = 0;
    statTime = 0;
    physMs = 0;
  }

  constexpr int offset = 25;
  DrawText(std::format("scene: {}", scene).c_str(), 25, 25, 15, RAYWHITE);
  DrawText(std::format("reg spf: {:.2f}", regAvg).c_str(), 25, 50, 15, RAYWHITE);
  DrawText(std::format("fst spf: {:.2f}", fastAvg).c_str(), 25, 50 + offset, 15, RAYWHITE);
  DrawText(std::format("tunnels: {}", tunnelCount).c_str(), 25, 50 + offset * 2, 15, RAYWHITE);
  DrawText(std::format("ccd hit: {}", ccdHits).c_str(), 25, 50 + offset * 3, 15, RAYWHITE);
  DrawText(std::format("phys: {:.4f} mcspf {:.4f} mcsps", msPerFrame * 1000, msPerStep * 1000).c_str(), 25, 50 + offset * 4, 15, RAYWHITE);
}

// moving b only if normal is from b to a
void World::CollidePair(Box &a, Box &b, bool moveB) {
  auto o = GetCollisionRec(a.AsRect(), b.AsRect());
  if (o.width <= 0 || o.height <= 0) // no collision
    return;

  auto ca = Vector2Add(a.position, Vector2Scale(a.extent, 0.5f));
  auto cb = Vector2Add(b.position, Vector2Scale(b.extent, 0.5f));
  bool xAxis = o.width < o.height;
  auto n = xAxis ? Vector2{ca.x < cb.x ? -1.f : 1.f, 0} : Vector2{0, ca.y < cb.y ? -1.f : 1.f};
  float depth = xAxis ? o.width : o.height;

  float wa = 1.f / a.mass;
  float wb = 1.f / b.mass;
  float share = moveB ? wa / (wa + wb) : 1.f;
  a.position = Vector2Add(a.position, Vector2Scale(n, depth * share));
  if (moveB)
    b.position = Vector2Subtract(b.position, Vector2Scale(n, depth * (1 - share)));

  float vn = Vector2DotProduct(Vector2Subtract(a.velocity, b.velocity), n);
  if (vn >= 0)
    return;
  float j = -(1 + restitution) * vn / (wa + wb);
  a.velocity = Vector2Add(a.velocity, Vector2Scale(n, j * wa));
  b.velocity = Vector2Subtract(b.velocity, Vector2Scale(n, j * wb));
}
