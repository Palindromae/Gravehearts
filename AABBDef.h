#pragma once
#include <glm/glm.hpp>

struct AABBDef
{
  glm::vec3 minimum{0, 0, 0};  // Aabb
  glm::vec3 maximum{0, 0, 0};  // Aabb
};