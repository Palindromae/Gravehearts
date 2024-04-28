#pragma once
#include <glm/glm.hpp>

float manhattenDistance(glm::ivec3 a);
float manhattenDistance(glm::vec3 a);
float manhattenDistance(glm::vec2 a);

float ChebyshevDistance(glm::ivec3 a);
float ChebyshevDistance(glm::vec3 a);

float EuclideanDistance(glm::vec3 a);