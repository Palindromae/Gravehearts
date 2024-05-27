#pragma once
#include <vulkan/vulkan_core.h>
#include <nvvk/raytraceKHR_vk.hpp>
#include "EntityManager.h"
#include <glm/gtc/matrix_transform.hpp>
#include "EntityGPUStruct.h"
#include "PhysicsComponent.h"
class Entity
{
	VkAccelerationStructureInstanceKHR* data{}; // Pointer into array
	EntityGPUStruct* gpu_vars;
	glm::vec4 rotation{ };
	glm::vec3 scale{};

	int EntityID = -1;

public:
	Entity(glm::vec3 pos, int model, glm::vec4 rotation = glm::vec4(0), glm::vec3 scale = glm::vec3(1), PhysicsComponent physics_component = {});

	void SetPhysicsComponent(PhysicsComponent component);

	void SetTRS();

	// POSITION
	void Translate(glm::vec3 translate);
	void SetPosition(glm::vec3 position);
	glm::vec3 Get_Position();

	// ROTATION


	void Rotate(glm::vec4 rotate);
	void SetRotation(glm::vec4 rotation);
	glm::vec3 Get_Rotation();

	//SCALE

	void Scale(glm::vec3 scale);
	void SetScale(glm::vec3 scale);
	glm::vec3 Get_Scale();

	int ID_Get();




	~Entity();
};