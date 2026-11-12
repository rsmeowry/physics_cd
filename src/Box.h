#ifndef PHYSICS_CD_BOX_H
#define PHYSICS_CD_BOX_H
#include <cstdint>


#include "raylib.h"


struct Box {
  Vector2 prevPosition;
  Vector2 position;
  Vector2 extent;
  Vector2 velocity;
  float mass;
  Color color = RED;
  uint32_t lifetime = 0;

  Box(Vector2 position, Vector2 extent, float mass, Color color) {
    prevPosition = position;
    this->position = position;
    this->extent = extent;
    this->mass = mass;
    this->color = color;
  }
};


#endif // PHYSICS_CD_BOX_H
