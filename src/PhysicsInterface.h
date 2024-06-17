#pragma once
#include <exception>
#include <glm/glm.hpp>
#include "Entity.h"
#include "EntityFrameData.h"

class PhysicsManager;
#define NVEPhysics PhysicsInterface::PhysInterface

class PhysicsInterface {
public:
	inline static PhysicsInterface* PhysInterface{};
private:
	PhysicsManager* Manager;


public:
	PhysicsInterface();
	void AddPhysicsObject(Entity* entity);
	void RemovePhysicsObject(Entity* entity);
	EntityFrameData* GetCurrentDefinedPhysicsFrame();
	void InitiateNewPhysicsUpdate(bool WaitTilLastPhysicsEnded);

	void SetEntityActivity(int id, bool status);

	void SetEntityInactive(int id);

	void SetEntityActive(int id);

};