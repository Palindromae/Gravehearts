#include "Entity.h"
#include "PhysicsInterface.h"
#include <algorithm>

Entity::Entity(glm::vec3 pos, int model, glm::quat rotation, glm::vec3 scale, PhysicsComponent physics_component) : scale(scale) {
	data = EntityManager::instance->GetID(EntityID);
	dataptr = EntityManager::instance->GetDataPtr(position,this->rotation,this->model);

	*position = pos;
	*this->model = model;
	*this->rotation = rotation;
	NVEPhysics->AddPhysicsObject(this);
	NVEPhysics->SetEntityActive(EntityID);

	SetTRS();

	data->instanceCustomIndex = EntityID;
	data->mask = 0xff;
	data->accelerationStructureReference = EntityManager::instance->GetModelBlasPtr(model);
	data->flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
	data->instanceShaderBindingTableRecordOffset = 0;  // We will use the same hit group for all objects
}



void Entity::SetTRS() {
	//glm::mat4 rotation_x = glm::mat4{ { 1,0,0,0 },{ 0, glm::cos(rotation.x), glm::sin(rotation.x),0 },{ 0, -glm::sin(rotation.x),glm::cos(rotation.x), 0 },{ 0, 0, 0, 1 } };
	//glm::mat4 rotation_y = glm::mat4{ { glm::cos(rotation.y),0,-glm::sin(rotation.y),0 },{ 0,1,0,0 },{ glm::sin(rotation.y), 0, glm::cos(rotation.y), 0 },{ 0, 0, 0, 1 } };
	//glm::mat4 rotation_z = glm::mat4{ { glm::cos(rotation.z),glm::sin(rotation.z),0,0 },{ -glm::sin(rotation.z), glm::cos(rotation.z), 0, 0 },{ 0, 0, 1, 0 },{ 0, 0, 0, 1 } };
	//glm::mat4 translate_scale = glm::mat4{ { scale.x,0,0,pos.x },{ 0,scale.y, 0, pos.y },{ 0, 0, scale.z, pos.z },{ 0, 0, 0, 1 } };


	glm::mat4 mat = glm::translate(glm::mat4(1), *position);
	//mat = mat * glm::rotate(glm::mat4(1), 1.0f, glm::vec3(0));
	//mat = mat * glm::scale(glm::mat4(1), scale);
	data->transform = nvvk::toTransformMatrixKHR(mat);

	//data->transform = nvvk::toTransformMatrixKHR(glm::mat4(1));
}

// POSITION
/*
void Entity::Translate(glm::vec3 translate)
{
	*position += translate;

	SetTRS();

}

void Entity::SetPosition(glm::vec3 position) {

	*this->position = position;

	SetTRS();
}
*/

glm::vec3 Entity::Get_Position() {
	return *position;
}

/*
void Entity::Rotate(glm::quat rotate) {
	*rotation *= rotate;

	SetTRS();
}

void Entity::SetRotation(glm::quat rotation) {
	*this->rotation = rotation;

	SetTRS();
}
*/

glm::quat Entity::Get_Rotation() {
	return *rotation;
}

void Entity::Scale(glm::vec3 scale) {
	this->scale *= scale;

	SetTRS();
}

void Entity::SetScale(glm::vec3 scale) {
	this->scale = scale;

	SetTRS();
}

glm::vec3 Entity::Get_Scale() {
	return scale;
}

int Entity::ID_Get() {
	return EntityID;
}

Entity::~Entity() {
	
	// Remove Physics Object
	NVEPhysics->RemovePhysicsObject(this);
	NVEPhysics->SetEntityInactive(EntityID);

	data->mask = 0x00; //Invalidate data
	data->transform = {};
	EntityManager::instance->ReturnID(data);


}
