#pragma once
#include <exception>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <vector>
#include "Entity.h"
#include "EntityFrameData.h"
#include "Physics_TranslationCommand.h"
#include "Physics_RotationCommand.h"
#include "PhysicsComponentCommand.h"
class PhysicsManager;
#define NVEPhysics PhysicsInterface::PhysInterface

class PhysicsInterface {
public:
	inline static PhysicsInterface* PhysInterface{};
private:
	PhysicsManager* Manager;
public:
	PhysicsInterface();
	void AddPhysicsObject(const int id, PhysicsComponent component);
	void RemovePhysicsObject(const int id);
	EntityFrameData* GetCurrentDefinedPhysicsFrame();
	void InitiateNewPhysicsUpdate();

	//void SetEntityActivity(int id, bool status);
	//void SetEntityInactive(int id);
	//void SetEntityActive(int id);


//// TLS ALTERATIONS ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	bool * EntityChanged;
	bool * TLSChanged;

	// Position Changes
	TranslationCommand* Translations{};
	TranslationCommand* Velocity{};

	// Rotation Changes
	RotationCommand* Rotations{ };
	RotationCommand* AngularVelocity{ };
public:
	// Translation
	void InstantTranslate(const int id, const glm::vec3 translation);   // Moves entity by delta position
	void SetPosition(const int id, const glm::vec3 WorldSpacePosition); // Moves entity directly to the world space position

	// Rotation
	void InstantRotate(const int id, const  glm::quat rotation);
	void SetRotation(const int id, const glm::quat rotation);

	//Velocity
	void SetVelocity(const int id, const glm::vec3 velocity);
	void DeltaVelocity(const int id, const  glm::vec3 velocity);

	// Angular Velocity
	void SetAngularVelocity(const int id, const glm::quat velocity);
	void DeltaAngularVelocity(const int id, const glm::quat velocity);

	// Force application
	void ApplyForce(int id, glm::vec3 direction, float magnitude);
	void ApplyForceAtAngle(int id, glm::vec3 origin, glm::vec3 direction, float magnitude);

	// Scale
	void SetScale(int id, glm::vec3 scale);

//// ENTITY --- MODEL INTERACTION ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
private:
	int* Models;
	PhysicsComponentCommand** PhysicsComponents;
public:
	void SetModelNextFrame(const int id, const uint32_t modelID);

	void CreateEntity(const int id, const uint32_t modelID, const PhysicsComponent component, const glm::vec3 Position, const glm::quat Rotation);
	void DeleteEntity(int id);

	// Apply Alterations
	void ApplyChangesToNextFrame();
};