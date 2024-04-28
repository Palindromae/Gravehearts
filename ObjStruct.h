#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include "nvvk/resourceallocator_vk.hpp"

struct ObjModel
{
  uint32_t     nbIndices{0};
  uint32_t     nbVertices{0};
  nvvk::Buffer vertexBuffer;    // Device buffer of all 'Vertex'
  nvvk::Buffer indexBuffer;     // Device buffer of the indices forming triangles
  nvvk::Buffer matColorBuffer;  // Device buffer of array of 'Wavefront material'
  nvvk::Buffer matIndexBuffer;  // Device buffer of array of 'Wavefront material'
};

struct ObjInstance
{
  glm::mat4 transform;    // Matrix of the instance
  uint32_t  objIndex{0};  // Model index reference
};
