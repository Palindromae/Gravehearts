#include "PhysicsInterface.h"
#include "PhysicsManager.h"
#include "ModelConst.h"

PhysicsInterface::PhysicsInterface() {
	if (PhysInterface != nullptr)
		throw new std::exception("you cannot have more than one physics interfaces running at a time");

	PhysInterface = this;

	Translations    = new TranslationCommand[MaxEntities];
	Velocity        = new TranslationCommand[MaxEntities];
	Rotations       = new RotationCommand[MaxEntities];
	AngularVelocity = new RotationCommand[MaxEntities];

	EntityChanged = new bool[MaxEntities];
	TLSChanged = new bool[MaxEntities];

	Models = new int[MaxEntities];
	PhysicsComponents = new PhysicsComponentCommand*[MaxEntities];


	for (size_t id = 0; id < MaxEntities; id++)
	{
		Translations[id] = TranslationCommand();
		Velocity[id] = TranslationCommand();
		Rotations[id] = RotationCommand();
		AngularVelocity[id] = RotationCommand();

		EntityChanged[id] = false;
		TLSChanged[id] = false;
		
		Models[id] = NullModel;
		PhysicsComponents[id] = nullptr;
	}


	Manager = new PhysicsManager();
}

void PhysicsInterface::AddPhysicsObject(const int id, PhysicsComponent component) {
	if (PhysicsComponents[id] != nullptr)
		delete PhysicsComponents[id]; // overwrite previous command

	PhysicsComponents[id] = new PhysicsComponentCommand(component);
	EntityChanged[id] = true;
}

void PhysicsInterface::RemovePhysicsObject(const int id) {
	if (PhysicsComponents[id] != nullptr)
		delete PhysicsComponents[id]; // overwrite previous command

	PhysicsComponents[id] = new PhysicsComponentCommand({}, true);
	EntityChanged[id] = true;
	throw std::exception("Not implemented");
}

EntityFrameData* PhysicsInterface::GetCurrentDefinedPhysicsFrame() {
	return Manager->GetCurrentDefinedPhysicsFrame();
}

void PhysicsInterface::InitiateNewPhysicsUpdate()
{
	// Wait til frame ends and copy to TLS
	Manager->WaitForEndOfFrame();
	Manager->CopyResultsOutToTLS();
	
	// Alter the next frame by previous commands (wipes commands clean afterwards)
	ApplyChangesToNextFrame();

	// Queue next frame

	Manager->InitiateNewPhysicsUpdate();
}

//void PhysicsInterface::SetEntityActivity(int id, bool status) {
//	Manager->SetEntityActivity(id, status);
//}
//
//void PhysicsInterface::SetEntityInactive(int id)
//{
//	Manager->SetEntityInactive(id);
//}
//
//void PhysicsInterface::SetEntityActive(int id)
//{
//	Manager->SetEntityActive(id);
//}

void PhysicsInterface::InstantTranslate(const int id, const glm::vec3 translation)
{
	Translations[id].TranslationVector += translation;
	
	TotalChanges += !EntityChanged[id];
	EntityChanged[id] = true;
	TLSChanged[id] = true;
}

void PhysicsInterface::SetPosition(const int id, const glm::vec3 WorldSpacePosition)
{
	Translations[id].Delta = false;
	Translations[id].TranslationVector = WorldSpacePosition;
	
	TotalChanges += !EntityChanged[id];
	EntityChanged[id] = true;
	TLSChanged[id] = true;
}

void PhysicsInterface::InstantRotate(const int id, const glm::quat rotation)
{
	Rotations[id].RotationVector *= rotation;
	
	TotalChanges += !EntityChanged[id];
	EntityChanged[id] = true;
	TLSChanged[id] = true;
}

void PhysicsInterface::SetRotation(const int id, const  glm::quat rotation)
{
	Rotations[id].Delta = false;
	Rotations[id].RotationVector = rotation;
	
	TotalChanges += !EntityChanged[id];
	EntityChanged[id] = true;
	TLSChanged[id] = true;
}

void PhysicsInterface::SetVelocity(const int id, const glm::vec3 velocity)
{
	Velocity[id].Delta = false;
	Velocity[id].TranslationVector = velocity;
	
	TotalChanges += !EntityChanged[id];
	EntityChanged[id] = true;
	TLSChanged[id] = true;
}

void PhysicsInterface::DeltaVelocity(const int id, const glm::vec3 delta_velocity)
{
	Velocity[id].TranslationVector += delta_velocity;
	
	TotalChanges += !EntityChanged[id];
	EntityChanged[id] = true;
	TLSChanged[id] = true;
}

void PhysicsInterface::SetAngularVelocity(const int id, const  glm::quat velocity)
{
	Rotations[id].Delta = false;
	
	TotalChanges += !EntityChanged[id];
	Rotations[id].RotationVector = velocity;
	EntityChanged[id] = true;
	TLSChanged[id] = true;
}

void PhysicsInterface::DeltaAngularVelocity(const int id, const glm::quat velocity)
{
	Rotations[id].RotationVector *= velocity;
	
	TotalChanges += !EntityChanged[id];
	EntityChanged[id] = true;
	TLSChanged[id] = true;
}

void PhysicsInterface::SetModelNextFrame(const int id, const uint32_t modelID)
{
	Models[id] = modelID;
	
	TotalChanges += !EntityChanged[id];
	EntityChanged[id] = true;
}

void PhysicsInterface::CreateEntity(const int id, const uint32_t modelID, const PhysicsComponent component, const glm::vec3 Position, const glm::quat Rotation)
{
	SetModelNextFrame(id, modelID);
	SetPosition(id, Position);
	SetVelocity(id, glm::vec3(0));
	SetRotation(id, Rotation);
	AddPhysicsObject(id, component);
}

void PhysicsInterface::DeleteEntity(const int id)
{
	SetModelNextFrame(id, DeleteEntityCommand); 
	//RemovePhysicsObject(id); Is infered
}

void PhysicsInterface::ApplyChangesToNextFrame()
{
	int ii = 0;
	for (size_t id = 0; id < MaxEntities; id++)
	{

		if (!EntityChanged[id])
			continue;
		
		if (ii++ >= TotalChanges) // Skips if total changes has been exceeded. 
			return;

		// Check if the entity should be deleted
		if (Models[id] == DeleteEntityCommand)
		{
			Manager->SetEntityInactive(id);
			EntityManager::instance->SetModel(id, BlankModel);
			Manager->RemovePhysicsObject(id);
			continue;
		}

		// Set Model
		if (Models[id] != NullModel) {
			EntityManager::instance->SetModel(id, Models[id]);
			Models[id] = NullModel;
		}

		// Set TLS data
		if (TLSChanged[id]) {
			Manager->TLSAlteration(id, Translations[id], Velocity[id], Rotations[id], AngularVelocity[id]);
			TLSChanged[id] = false;
		}
		
		EntityChanged[id] = false;

		// Set Physics Components
		if (PhysicsComponents[id] != nullptr) {

			//if (PhysicsComponents[id]->Destroy) 
			//	Manager->RemovePhysicsObject(id);
			//else 
			// Should be impossible to delete the physics object without deleting the entity
			Manager->AddPhysicsObject(id, PhysicsComponents[id]->component);

			delete PhysicsComponents[id];
			PhysicsComponents[id] = nullptr;
		}
	}
}

