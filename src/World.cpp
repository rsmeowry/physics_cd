#include "World.h"

#include <algorithm>

void World::SimulateSlow() {
  for (auto &box: this->slow) {
    // js testing
    box.velocity.x += GetFrameTime() * gravity.x;
    box.velocity.y += GetFrameTime() * gravity.y;
    box.prevPosition = box.position;
    box.position.x += GetFrameTime() * box.velocity.x;
    box.position.y += GetFrameTime() * box.velocity.y;
    box.lifetime += 1;
  }

  // sweeping dead boxes
  auto got = std::remove_if(this->slow.begin(), this->slow.end(),
                        [this](Box b) {
                          return b.lifetime > SLOW_BOX_LIFETIME_SECS * this->regularSimRate;
                        });
  this->slow.erase(got, this->slow.end());
  this->regularStepsPerFrame += 1;
}
void World::SimulateFast() {
  for (auto &box: this->fast) {
    // js testing
    box.velocity.x += GetFrameTime() * gravity.x;
    box.velocity.y += GetFrameTime() * gravity.y;
    box.prevPosition = box.position;
    box.position.x += GetFrameTime() * box.velocity.x;
    box.position.y += GetFrameTime() * box.velocity.y;
    box.lifetime += 1;
  }

  // sweeping dead boxes
  auto got = std::remove_if(this->fast.begin(), this->fast.end(),
                        [this](Box b) {
                          return b.lifetime > FAST_BOX_LIFETIME_SECS * this->fastSimRate;
                        });
  this->fast.erase(got, this->fast.end());
  this->fastStepsPerFrame += 1;
}

void World::DrawBoxes() {
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
