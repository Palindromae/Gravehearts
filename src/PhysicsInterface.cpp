#include "PhysicsInterface.h"
#include "PhysicsManager.h"

PhysicsInterface::PhysicsInterface() {
	if (PhysInterface != nullptr)
		throw new std::exception("you cannot have more than one physics managers running at a time");

	PhysInterface = this;

	Manager = new PhysicsManager();
}

void PhysicsInterface::AddPhysicsObject(Entity* entity) {
	Manager->AddPhysicsObject(entity);

}

void PhysicsInterface::RemovePhysicsObject(Entity* entity) {
	Manager->RemovePhysicsObject(entity);
}

EntityFrameData* PhysicsInterface::GetCurrentDefinedPhysicsFrame() {
	return Manager->GetCurrentDefinedPhysicsFrame();
}

void PhysicsInterface::InitiateNewPhysicsUpdate(bool WaitTilLastPhysicsEnded)
{
	Manager->InitiateNewPhysicsUpdate(WaitTilLastPhysicsEnded);
}

void PhysicsInterface::SetEntityActivity(int id, bool status) {
	Manager->SetEntityActivity(id, status);
}

void PhysicsInterface::SetEntityInactive(int id)
{
	Manager->SetEntityInactive(id);
}

void PhysicsInterface::SetEntityActive(int id)
{
	Manager->SetEntityActive(id);
}

