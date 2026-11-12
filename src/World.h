#ifndef PHYSICS_CD_WORLD_H
#define PHYSICS_CD_WORLD_H
#include <format>
#include <vector>

#include "Box.h"

#define FPS 144
#define FAST_BOX_LIFETIME_SECS 8
#define SLOW_BOX_LIFETIME_SECS 16

enum SimulationMode {
  Discrete60,
  Multi240,
  CCD60
};

class World {
private:
  std::vector<Box> statics { };
  std::vector<Box> slow { };
  std::vector<Box> fast { };
  Vector2 gravity{ 0, 980 };

  float restitution = 0.5f;
  SimulationMode simulationMode = Discrete60;

  // stats
  int regularStepsPerFrame = 0;
  int fastStepsPerFrame = 0;
  int tunnelCount = 0;
  int ccdHits = 0;
  int statFrames = 0;
  float statTime = 0;
  float regAvg = 0;
  float fastAvg = 0;

  double physMs = 0;
  double msPerFrame = 0;
  double msPerStep = 0;

  void SimulateFast(float dt);
  void SimulateSlow(float dt);
  bool SweepAABB(Box& a, Vector2 dir, Box& b, float& tHit, Vector2& normal);
  void SimulateCCD(Box& box, float dt);
public:
  void SetMode(SimulationMode simulationMode) {
    this->simulationMode = simulationMode;
    switch (simulationMode) {
      case Discrete60:
        this->fastSimRate = 60;
        this->regularSimRate = 60;
        break;
      case Multi240:
        this->fastSimRate = 240;
        this->regularSimRate = 60;
        break;
      case CCD60:
        this->fastSimRate = 60;
        this->regularSimRate = 60;
        break;
    }
  }

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
  void Clear() {
    statics.clear();
    fast.clear();
    slow.clear();
    ccdHits = 0;
    tunnelCount = 0;
    physMs = 0;
    msPerFrame = 0;
    msPerStep = 0;
  }

  void Simulate(float dt);
  void DrawBoxes(float alpha);
  void DrawStats();
  void CollideStatics(Box& box);
};


#endif // PHYSICS_CD_WORLD_H
