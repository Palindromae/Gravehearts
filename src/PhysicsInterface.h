#pragma once
#include <exception>
#include <glm/glm.hpp>
#include "Entity.h"
#include "EntityFrameData.h"

class PhysicsManager;

class PhysicsInterface {
public:
	inline static PhysicsInterface* Physics{};
private:
	PhysicsManager* Manager;

	PhysicsInterface();

public:

	void AddPhysicsObject(Entity* entity);
	void RemovePhysicsObject(Entity* entity);
	EntityFrameData* GetCurrentDefinedPhysicsFrame();
	void InitiateNewPhysicsUpdate(bool WaitTilLastPhysicsEnded);
};