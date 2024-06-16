#include "PhysicsInterface.h"
#include "PhysicsManager.h"

PhysicsInterface::PhysicsInterface() {
	if (Physics != nullptr)
		throw new std::exception("you cannot have more than one physics managers running at a time");

	Physics = this;

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
