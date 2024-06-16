#include "PhysicsManager.h"
#include "CurrentEntityTLSData.h"

void PhysicsManager::PhysicsUpdate() {
	int partitionID = 0;
	ComputeBuffer* Models;
	PhysicsPartition* partition = &partitions[partitionID];
	EntityManager::instance->GetEntityArr(Models);

	// Initiate Velocities -- Apply Forces, Apply Current Velocities
	int* ActiveEntities;
	int ActiveEntitiesNO;

	ApplyForcesToActiveEntities(ActiveEntitiesNO, ActiveEntities);

	/// Apply Restraints



	// Find Possible Collisions
	auto PossibleCollisions = SATCollisionDetection(partitionID);

	// Create Collision Queries
	std::vector<SATCollision*> ColliderOnCollider{};
	std::vector<PhysicsCubeTraceInfo> ModelOnX{};
	int i = 0;
	for (auto& PCol : PossibleCollisions)
	{

		if (PhysicsComponents[PCol.A_ID].collderType == ColliderType::Virtual && PhysicsComponents[PCol.B_ID].collderType == ColliderType::Virtual) { // Colliders are both virtual, so only use the SAT collision
			ColliderOnCollider.push_back(&PCol);
			continue;
		}

		glm::vec3 rel_velocity = CurrentFrame.VelocityBuffer[PCol.B_ID] - CurrentFrame.VelocityBuffer[PCol.A_ID];
		glm::vec3 normal = glm::normalize(rel_velocity);
		glm::quat rotationAtIncidenceA = glm::fastMix(PastFrame.RotationBuffer[PCol.A_ID], CurrentFrame.RotationBuffer[PCol.A_ID], PCol.timeToTarget / FixedDeltaTime);
		glm::quat rotationAtIncidenceB = glm::fastMix(PastFrame.RotationBuffer[PCol.B_ID], CurrentFrame.RotationBuffer[PCol.B_ID], PCol.timeToTarget / FixedDeltaTime);

		float A_Size = CalculateShadowCubeSize(normal * rotationAtIncidenceA, PCol.A_ID);
		float B_Size = CalculateShadowCubeSize(normal * rotationAtIncidenceB, PCol.B_ID);

		PhysicsCubeTraceInfo trace{};
		if (A_Size < B_Size)
		{
			trace.positionA = PastFrame.PositionBuffer[PCol.A_ID] + CurrentFrame.VelocityBuffer[PCol.A_ID] * PCol.timeToTarget;
			trace.RotationA = { rotationAtIncidenceA.x,rotationAtIncidenceA.y, rotationAtIncidenceA.z,rotationAtIncidenceA.w };
			trace.ModelTypeA = (int)PhysicsComponents[PCol.A_ID].collderType;

			trace.positionB = PastFrame.PositionBuffer[PCol.B_ID] + CurrentFrame.VelocityBuffer[PCol.B_ID] * PCol.timeToTarget;
			trace.RotationB = { rotationAtIncidenceB.x,rotationAtIncidenceB.y, rotationAtIncidenceB.z,rotationAtIncidenceB.w };
			trace.ModelTypeB = (int)PhysicsComponents[PCol.B_ID].collderType;
		}
		else
		{

			// Prefer rays come from smaller target
			trace.positionB = PastFrame.PositionBuffer[PCol.A_ID] + CurrentFrame.VelocityBuffer[PCol.A_ID] * PCol.timeToTarget;
			trace.RotationB = { rotationAtIncidenceA.x,rotationAtIncidenceA.y, rotationAtIncidenceA.z,rotationAtIncidenceA.w };
			trace.ModelTypeB = (int)PhysicsComponents[PCol.A_ID].collderType;

			trace.positionA = PastFrame.PositionBuffer[PCol.B_ID] + CurrentFrame.VelocityBuffer[PCol.B_ID] * PCol.timeToTarget;
			trace.RotationA = { rotationAtIncidenceB.x,rotationAtIncidenceB.y, rotationAtIncidenceB.z,rotationAtIncidenceB.w };
			trace.ModelTypeA = (int)PhysicsComponents[PCol.B_ID].collderType;
		}

		//trace_infos[i].quality = SizeToQuality(size);
		trace.direction = normal;
		trace.MaxDistance = (FixedDeltaTime - PCol.timeToTarget) * glm::dot(rel_velocity, normal);
		ModelOnX.push_back(trace);
		i++;
	}

	PhysicsVec3Data->setBufferData(PastFrame.PositionBuffer, PhysicsVec3::Position_PastFrame * MaxEntities, MaxEntities, context);
	PhysicsVec3Data->setBufferData(CurrentFrame.PositionBuffer, PhysicsVec3::Velocity_PastFrame * MaxEntities, MaxEntities, context);

	PhysicsVec4Data->setBufferData(CurrentFrame.RotationBuffer, PhysicsVec4::Rotation_NextFrame * MaxEntities, MaxEntities, context);

	EntityActiveMasksBuffer->setBufferData(EntityActiveMasksBuffer, 0, EntityActiveMasksBuffer->info.length, context);

	// Apply Initial Phyiscs and Send out collision "feelers"


	// Solve Collisions
	// Prevent Ground Collision

	// Send out inital rays

	/// Resolve Collisions

	// Resolve Virtual Collisions
	for (SATCollision* VCol : ColliderOnCollider)
	{
		ResolvePositionAfterCollision(FixedDeltaTime, VCol->PointOfContact, VCol->MinimumTranslationVector, glm::normalize(VCol->MinimumTranslationVector), PhysicsComponents[VCol->A_ID], VCol->A_ID, PhysicsComponents[VCol->B_ID], VCol->B_ID, CurrentFrame.PositionBuffer, CurrentFrame.VelocityBuffer);
	}


	PhysicsRayCollision* collisions = (PhysicsRayCollision*)CollisionsCallback->GetMemoryCopy(); //Potentially have a int storing the number of collisions

	//for (size_t i = 0; i < ModelOnX.size(); i++)
	//{
	// If the ObjB is closer than the world, collide with it
	//if(collisions[i].distanceToPosition < collisions[i + PossibleCollisions.size()].distanceToPosition)
	//{
	//	if(collisions[i].ObjBID == NoCollision)
	//			continue;
	// Hit ObjB
	//	ResolvePositionAfterCollision(FixedDeltaTime,,,collisions[i].normal,PhysicsComponents[trace_infos[i].ID], trace_infos[i].ID, collisions->objA_position,PhysicsComponents[collisions[i].ObjBID], collisions[i].ObjBID, collisions[i].objB_position, CurrentFrame.PositionBuffer,CurrentFrame.VelocityBuffer);
	//	}
	//	else if (collisions[i + PossibleCollisions.size()].ObjBID != NoCollision) {
	// Hit World

	//	if (collisions[i].ObjBID == NoCollision)
	//					continue;


	//	}

	//}




	// Find World Collisions
	int temp;
	Shaders::Shaders[Shaders::ObjectWorldCollisionDetection]->dispatch(context, MaxEntities, 1, 1, sizeof(int), &temp, MonoidList(4).bind(PhysicsVec3Data)->bind(PhysicsVec3Data)->bind(EntityActiveMasksBuffer)->bind(Models)->bind(ModelManager::instance->modelBuffer)->bind(ChunkTlas)->render());
	//DisjointDispatcher->WaitOnOneFenceMax(&context->fence);

	for (size_t i = 0; i < ActiveEntitiesNO; i++)
	{
		int j = ActiveEntities[i];
		if (collisions[j].ObjBID == NoCollision)
			continue;

		ResolvePositionAfterCollision_World(collisions[j], PhysicsComponents[j], j, CurrentFrame.PositionBuffer, CurrentFrame.VelocityBuffer);
	}


	// INTEGRATE --- Update Angular Velocity and Velocity 
	for (size_t i = 0; i < ActiveEntitiesNO; i++)
	{
		UpdateVelocity(FixedDeltaTime, ActiveEntities[i], CurrentFrame.VelocityBuffer, CurrentFrame.PositionBuffer, PastFrame.PositionBuffer);
		UpdateAngularVelocity(FixedDeltaTime, ActiveEntities[i], CurrentFrame.AngularVelocityBuffer, CurrentFrame.RotationBuffer, PastFrame.RotationBuffer);
	}

	// Resolve Collision Velocities

	for (const auto& VCol : ColliderOnCollider)
	{
		ResolveVelocitiesAfterCollision(
			FixedDeltaTime, VCol->ADist, VCol->PointOfContact, glm::normalize(VCol->MinimumTranslationVector),
			CurrentFrame.PositionBuffer, CurrentFrame.RotationBuffer, CurrentFrame.VelocityBuffer, CurrentFrame.AngularVelocityBuffer,
			PastFrame.PositionBuffer, PastFrame.RotationBuffer, PastFrame.VelocityBuffer, PastFrame.AngularVelocityBuffer,
			PhysicsComponents[VCol->A_ID], VCol->A_ID, PhysicsComponents[VCol->B_ID], VCol->B_ID);

	}

	for (size_t i = 0; i < ActiveEntitiesNO; i++)
	{
		int j = ActiveEntities[i];

		PhysicsRayCollision collision = collisions[j];

		if (collision.ObjBID == NoCollision)
			continue;

		//ResolveVelocitiesAfterCollision_World
		ResolveVelocitiesAfterCollision(
			FixedDeltaTime, collision.distanceToPosition, collision.objA_position, collision.normal,
			CurrentFrame.PositionBuffer, CurrentFrame.RotationBuffer, CurrentFrame.VelocityBuffer, CurrentFrame.AngularVelocityBuffer,
			PastFrame.PositionBuffer, PastFrame.RotationBuffer, PastFrame.VelocityBuffer, PastFrame.AngularVelocityBuffer,
			PhysicsComponents[j], j, PhysicsComponents[collision.ObjBID], collision.ObjBID);
	}

	// Check Activity of Entities

	for (size_t i = 0; i < ActiveEntitiesNO; i++)
	{

		inactive_time[ActiveEntities[i]] += int(glm::dot(CurrentFrame.VelocityBuffer[ActiveEntities[i]], CurrentFrame.VelocityBuffer[ActiveEntities[i]]) < minVelocity) * FixedDeltaTime;

		int mask = 0;
		int bump = (ActiveEntities[i] & (ENTITYCHUNK_SIZE - 1));
		mask |= 1 << bump;

		int entityChunk = ActiveEntities[i] / ENTITYCHUNK_SIZE;
		active_masks[entityChunk] = (active_masks[entityChunk] & ~mask) | ((inactive_time[ActiveEntities[i]] > IDLETIME) << bump);
	}


	// Wipe Collisions data

	free(ActiveEntities);
}

void PhysicsManager::InitiateNewPhysicsUpdate(bool WaitTilLastPhysicsEnded) {

	while (WaitTilLastPhysicsEnded)
	{
		if (Status_Get() == ThreadStatus::Ready)
			break;
	}

	//SetCurrentFrame.Copy(CurrentFrame);

	TLS::PreviousInterpolationFrame->Copy(PastFrame);
	PastFrame.Copy(CurrentFrame);

	newPhysicsFrame = true;

}
