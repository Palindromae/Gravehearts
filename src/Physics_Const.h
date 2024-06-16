#pragma once

const float FixedDeltaTime = static_cast<float>(1 / 30.0); // up to 1/90
const float MaxQualityReductionToScanCollisions = 1;
const glm::ivec3 PhysicsChunkSize = glm::ivec3(8);

const float WorldStaticFriction = .5f;
const float WorldDynamicFriction = .2f;