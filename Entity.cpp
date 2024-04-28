#include "Entity.h"

Entity::Entity(glm::vec3 pos, int model, glm::vec4 rotation, glm::vec3 scale) : rotation(rotation), scale(scale) {
	int no = -1;
	data = EntityManager::instance->GetID(no);
	gpu_vars = EntityManager::instance->GetDataPtr();

	gpu_vars->position = pos;
	gpu_vars->model = model;

	SetTRS();

	data->instanceCustomIndex = no;
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


	glm::mat4 mat = glm::translate(glm::mat4(1), gpu_vars->position);
	//mat = mat * glm::rotate(glm::mat4(1), 1.0f, glm::vec3(0));
	//mat = mat * glm::scale(glm::mat4(1), scale);
	data->transform = nvvk::toTransformMatrixKHR(mat);

	//data->transform = nvvk::toTransformMatrixKHR(glm::mat4(1));
}

// POSITION

void Entity::Translate(glm::vec3 translate)
{
	gpu_vars->position += translate;

	SetTRS();

}

void Entity::SetPosition(glm::vec3 position) {

	gpu_vars->position = position;

	SetTRS();
}

glm::vec3 Entity::Get_Position() {
	return gpu_vars->position;
}

void Entity::Rotate(glm::vec4 rotate) {
	rotation *= rotate;

	SetTRS();
}

void Entity::SetRotation(glm::vec4 rotation) {
	this->rotation = rotation;

	SetTRS();
}

glm::vec3 Entity::Get_Rotation() {
	return rotation;
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

Entity::~Entity() {
	data->mask = 0x00; //Invalidate data
	data->transform = {};
	EntityManager::instance->ReturnID(data);
}
