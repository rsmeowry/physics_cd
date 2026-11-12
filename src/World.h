#ifndef PHYSICS_CD_WORLD_H
#define PHYSICS_CD_WORLD_H
#include <format>
#include <vector>

#include "Box.h"

#define FPS 144
#define FAST_BOX_LIFETIME_SECS 8
#define SLOW_BOX_LIFETIME_SECS 16

class World {
private:
  std::vector<Box> statics { };
  std::vector<Box> slow { };
  std::vector<Box> fast { };
  Vector2 gravity{ 0, 980 };

  float restitution = 0.5f;

  // stats
  int regularStepsPerFrame = 0;
  int fastStepsPerFrame = 0;
  int tunnelCount = 0;
  int ccdHits = 0;

  void SimulateFast(float dt);
  void SimulateSlow(float dt);
public:
  int regularSimRate = 60;
  int fastSimRate = 240;

  void SpawnStatic(Box box) {
    statics.push_back(box);
  }
  void SpawnSlow(Box box) {
    slow.push_back(box);
  }
  void SpawnFast(Box box) {
    fast.push_back(box);
  }

  void Simulate(float dt);
  void DrawBoxes(float alpha);
  void DrawStats();
  void CollideStatics(Box& box);
};


#endif // PHYSICS_CD_WORLD_H
