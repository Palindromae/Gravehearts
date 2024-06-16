#ifdef __cplusplus
#pragma once
#include <glm/glm.hpp>
// GLSL Type
using vec3 = glm::vec3;
using ivec3 = glm::ivec3;
using uint = unsigned int;
#endif
const int MaxEntities = 1200;


// clang-format off
#ifdef __cplusplus // Descriptor binding helper for C++ and GLSL
#define START_BINDING(a) enum a {
#define END_BINDING() }
#else
#define START_BINDING(a)  const uint
#define END_BINDING() 
#endif

START_BINDING(PhysicsVec3)
Position_PastFrame = 0,
Position_NextFrame = 1,  // Current frame that Physics is calculating
Velocity_PastFrame = 2,
Velocity_NextFrame = 3,   
END
END_BINDING();

START_BINDING(PhysicsVec4)
Rotation_PresentFrame = 0,  
Rotation_NextFrame = 1,    
AngularVelocity_PresentFrame = 2,   
AngularVelocity_NextFrame = 3,   
END
END_BINDING();

START_BINDING(EntityVec3)
Position_Interpolated = 0,
END
END_BINDING();

START_BINDING(EntityVec4)
Rotation_Interpolated,
END
END_BINDING();