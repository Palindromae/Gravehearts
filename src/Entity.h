#pragma once
#include <vulkan/vulkan_core.h>
#include <nvvk/raytraceKHR_vk.hpp>
#include "EntityManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include "Physics_Component.h"
class Entity
{
	VkAccelerationStructureInstanceKHR* data{}; // Pointer into array

	// Data
	int dataptr;
	glm::vec3* position;
	glm::quat* rotation;
	int* model;
	glm::vec3 scale{};

	int EntityID = -1;

public:
	Entity(int model, glm::vec3 pos, glm::quat rotation = glm::quat(), glm::vec3 scale = glm::vec3(1), PhysicsComponent physics_component = PhysicsComponent());

	void SetPhysicsComponent(PhysicsComponent component);

	void SetTRS();

	// POSITION
	void Translate(glm::vec3 translate);
	void SetPosition(glm::vec3 position);
	glm::vec3 Get_Position();

	// ROTATION


	void Rotate(glm::quat rotate);
	void SetRotation(glm::quat rotation);
	glm::quat Get_Rotation();

	//SCALE

	void Scale(glm::vec3 scale);
	void SetScale(glm::vec3 scale);
	glm::vec3 Get_Scale();

	int ID_Get();




	~Entity();
};