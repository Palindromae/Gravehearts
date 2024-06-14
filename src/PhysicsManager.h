#pragma once
#include "Physics_Component.h"
#include "../shaders/EntityConst.h"
#include "ThreadStatus.h"
#include "Physics_Partition.h"
#include "CallbackGPUMemory.h"
#include <immintrin.h>
#include <mutex>
#include <thread>
#include <exception>
#include "../shaders/Physics_CubeTraceInfo.h"
#include "../shaders/ChunkGPUConst.h"
#include "EntityManager.h"
#include "Physics_Const.h"
#include "../shaders/Physics_RayCollision.h"

#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

#include "StoredComputeShaders.h"
#include "Physics_Collisions.h"
#include "MonoidList.h"
#include "ModelManager.h"
class PhysicsManager {
	PhysicsComponent* PhysicsComponents{};
	Entity* PhysicsEntities{};

	std::jthread thread;
	ThreadStatus status = ThreadStatus::Starting;
	std::mutex status_mutex{};
	nve::ProductionPackage* context;

	///////// For SIMD



	uint32_t* active_masks{};
	float* inactive_time;


	glm::vec3* Entity_Position_Past;
	glm::vec3* Entity_Position_Updated;
	glm::vec3* Entity_Velocity;
	glm::vec3* Entity_Velocity_Past;

	glm::quat* Entity_Rotation_Past;
	glm::quat* Entity_Rotation_Updated;
	glm::quat* Entity_Angular_Velocity;
	glm::quat* Entity_Angular_Velocity_Past;

	ComputeBuffer* EntityActiveMasksBuffer;

	//	glm::vec3* Collider_Position_WorldSpace;
	glm::vec3* Collider_Dimension_WorldSpace;

	///////
	//	glm::vec3* WorldPositionsToWork;
	//	glm::vec3* WorldDimensionsToWork;
	//////// Collisions

	ComputeBuffer* Collision_Information;
	ComputeBuffer* RayCollisions;
	CallbackGPUMemory* CollisionsCallback;
	////////
	
	const int ENTITYCHUNK_SIZE = sizeof(int);
	const int IDLETIME = 5; //Time Taken to become inactive.
	const float minVelocity = 0.001;

		// Map in place until we add in space partitioning
	PhysicsPartition partitions[8];
	bool newPhysicsFrame = false;

	int PartitionUIDToIndex(int x, int y, int z) {
		return x + y * 2 + z * 4;
	}


	int GetPartition(glm::vec3 position) {
		position /= PhysicsChunkSize;

		return PartitionUIDToIndex(int(position.x) & 2, int(position.y) & 2, int(position.z) & 2);
	}

	void main_physics()
	{
		while (true)
		{
			if (newPhysicsFrame) {
				newPhysicsFrame = false;

				PhysicsUpdate();
			}

		}
	}

	float CalculateShadowCubeSize(const glm::vec3 direction_rotated_to_obj_space, const uint32_t ID) {

		vec3 dimensions = PhysicsComponents[ID].dimensions / glm::vec3(2);

		float lowHigh_X[2] = { -dimensions.x, dimensions.x };
		float lowHigh_Y[2] = { -dimensions.y, dimensions.y };
		float lowHigh_Z[2] = { -dimensions.z, dimensions.z };


		glm::vec2 dot_x;
		glm::vec2 dot_y;
		glm::vec2 dot_z;

		dot_x.x = direction_rotated_to_obj_space.x * lowHigh_X[0];
		dot_x.y = -dot_x.x;
		dot_x = glm::max(glm::vec2(0), dot_x);

		dot_y.x = direction_rotated_to_obj_space.y * lowHigh_Y[0];
		dot_y.y = -dot_y.x;
		dot_y = glm::max(glm::vec2(0), dot_y);

		dot_z.x = direction_rotated_to_obj_space.z * lowHigh_Z[0];
		dot_z.y = -dot_z.x;
		dot_z = glm::max(glm::vec2(0), dot_z);

		float CollisionArea = 0;
		// X+ X-
		glm::vec2 areaTemp = glm::vec2(PhysicsComponents[ID].dimensions.y * PhysicsComponents[ID].dimensions.z) * dot_x;
		CollisionArea += areaTemp.x + areaTemp.y;
		
		// Y+ Y-
		areaTemp = glm::vec2(PhysicsComponents[ID].dimensions.x * PhysicsComponents[ID].dimensions.z) * dot_y;
		CollisionArea += areaTemp.x + areaTemp.y;
			
		// Z+ Z-
		areaTemp = glm::vec2(PhysicsComponents[ID].dimensions.x * PhysicsComponents[ID].dimensions.y) * dot_z;
		CollisionArea += areaTemp.x + areaTemp.y;

		return CollisionArea;
	}

	void UpdateEntityChunk(int i) {

		for (size_t j = 0; j < ENTITYCHUNK_SIZE; j++)
		{
			//Collider_Position_WorldSpace [i + j] = PhysicsEntities[j + i].Get_Position();
			Collider_Dimension_WorldSpace[i + j] = PhysicsEntities[j + i].Get_Rotation() * PhysicsComponents[j + i].dimensions;
		}

		/*	for (size_t j = 0; j < 32; j++)
			{
				WorldPositionsToWork[j] = PhysicsEntities[j + i].Get_Position() + PhysicsComponents[j + i].origin;
				WorldDimensionsToWork[j] = PhysicsEntities[j + i].Get_Rotation() + PhysicsComponents[j + i].dimension;
			}*/
	}

	void ApplyGravity(float deltaTime, int ID) {
		Entity_Position_Updated[ID] += PhysicsComponents[ID].Gravity * deltaTime;
	}
	void ApplyVelocity(float deltaTime, int ID) {
		Entity_Position_Updated[ID] += Entity_Velocity[ID] * deltaTime;
	}
	void ApplyAngularVelocity(float deltaTime, int ID) {
		Entity_Rotation_Updated[ID] = Entity_Rotation_Updated[ID] * Entity_Angular_Velocity[ID] * deltaTime;
	}

	bool isEntityActive(int id) {
		int chunk = id / ENTITYCHUNK_SIZE;
		int sub_id = id & (ENTITYCHUNK_SIZE - 1);

		return (active_masks[chunk] >> sub_id) & 1;
	}

	int numberOfSetBits(uint32_t i)
	{
		// Java: use int, and use >>> instead of >>. Or use Integer.bitCount()
		// C or C++: use uint32_t
		i = i - ((i >> 1) & 0x55555555);        // add pairs of bits
		i = (i & 0x33333333) + ((i >> 2) & 0x33333333);  // quads
		i = (i + (i >> 4)) & 0x0F0F0F0F;        // groups of 8
		i *= 0x01010101;                        // horizontal sum of bytes
		return  i >> 24;               // return just that top byte (after truncating to 32-bit even when int is wider than uint32_t)
	}


	bool SATCollisionAB(const int objI, const int objJ, SATCollision& collisionInfo) {
		if (!(isEntityActive(objI) || isEntityActive(objJ))) // if either are active then allow the collision. Else neither are moving, skip!
			return false;

		float maxI = std::max(PhysicsComponents[objI].dimensions.x, PhysicsComponents[objI].dimensions.y, PhysicsComponents[objI].dimensions.z);
		float maxJ = std::max(PhysicsComponents[objJ].dimensions.x, PhysicsComponents[objJ].dimensions.y, PhysicsComponents[objJ].dimensions.z);

		if (glm::distance2(Entity_Position_Updated[objI], Entity_Position_Updated[objJ]) > (maxI + maxJ) * (maxI + maxJ))
			return false;

		collisionInfo.A_ID = objI;
		collisionInfo.B_ID = objJ;

		return SATOptimised(Entity_Position_Updated, Entity_Rotation_Updated, objI, PhysicsComponents[objI].dimensions, objJ, PhysicsComponents[objJ].dimensions, collisionInfo, Entity_Velocity[objI] - Entity_Velocity[objJ]);
	}
	 

	std::vector<SATCollision> SATCollisionDetection(int partitionID) {

		std::vector<SATCollision> collisions;
		auto partition = partitions[partitionID];

		// For each region in this octant
		for (auto const& [_, val] : partition.EntityMap)
		{
			// Step through each possible collision in the region
			int i = 0;
			for (auto it = val.begin(); it != val.end(); it++, i++)
			{
				auto jt = val.begin();
				for (size_t j = 0; j <= i; j++)
					jt++; // step up to the current position to ensure a strict order

				for (; jt != val.end(); jt++)
				{
					auto iobj = it->first;
					auto jobj = jt->first;

					SATCollision collision{};
					if (!SATCollisionAB(iobj, jobj, collision))
						continue;

					collisions.push_back(collision);
				}
			}

		}
	
		
		/// For each entity in adjacent regions
		glm::ivec3 adjkey[8];
		int adjpartition[8];


		for (auto const& [key, val] : partition.EntityMap)
		{
			// For each partition region, calculate all offsets
			int i = 0;
			for (size_t x = 0; x < 2; x++)
			{
				for (size_t y = 0; y < 2; y++)
				{
					for (size_t z = 0; z < 2; z++)
					{
						adjkey[i] = (key + glm::ivec3(x, y, z)) &2;
						adjpartition[i] = PartitionUIDToIndex(adjkey[i].x, adjkey[i].y, adjkey[i].z);
						i++;
					}
				}
			}
			
			// For each entity within a region
			for (auto it = val.begin(); it != val.end(); it++)
			{
				for (size_t n = 0; n < 8; n++)
				{
					for (const auto& [id, val] : partitions[adjpartition[n]].EntityMap[adjkey[n]])
					{
						auto iobj = it->first;

						if (!(isEntityActive(iobj) || isEntityActive(id))) // if either are active then allow the collision. Else neither are moving, skip!
							continue;

						SATCollision collision{};
						if (SATCollisionAB(iobj, id, collision))
							collisions.push_back(collision);
					
					}
				}
			}

		}





		

	}


	void ApplyForcesToActiveEntities(int& NoActiveEntities, int*& ActiveEntities)
	{
		std::vector<int> ActiveChunks{};
		ActiveChunks.reserve(100);

		// Update Entity Chunks
		int NoActiveEntities = 0;
		int j = 0;
		for (size_t i = 0; j < MaxEntities; i++, j += 32)
		{
			if (active_masks[i] == 0)
				continue;

			NoActiveEntities += numberOfSetBits(active_masks[i]);
			UpdateEntityChunk(j);


			ActiveChunks.push_back(i);


			//for (size_t k = 0; k < 32; k++)
			//{
			//	if ((active_masks[i] >> k) & 1)
			//		ActiveEntities.push_back(k + j);
			//}
		}


		// Calculate Entities to Update
		//PhysicsCubeTraceInfo* trace_infos = (PhysicsCubeTraceInfo*)malloc(sizeof(PhysicsCubeTraceInfo) * NoActiveEntities);
		int* activeIDs = (int*)malloc(sizeof(int) * NoActiveEntities);

		for (size_t i = 0; i < ActiveChunks.size(); i++)
		{

			uint32_t worked_mask = active_masks[ActiveChunks[i]];
			for (size_t k = 0; k < 32; k++)
			{

				int EntityOffset = worked_mask & ~(worked_mask - 1); // Get lowest bit
				worked_mask -= EntityOffset; //Remove it from temporary mask

				int ID = ActiveChunks[i] * sizeof(worked_mask) * 8 + EntityOffset;

				activeIDs[i] = ID;

				if (worked_mask == 0) // if empty skip the rest of the chunk
					break;
			}

		}

		// Initiate Velocities -- Apply Forces, Apply Current Velocities
		for (size_t i = 0; i < NoActiveEntities; i++)
		{
			ApplyGravity(FixedDeltaTime, activeIDs[i]);
			ApplyVelocity(FixedDeltaTime, activeIDs[i]);
			ApplyAngularVelocity(FixedDeltaTime, activeIDs[i]);
		}
	}

	void PhysicsUpdate() {
		int partitionID = 0;
		ComputeBuffer* Models;
		PhysicsPartition* partition = &partitions[partitionID];
		{
			glm::vec3* PositionBuffer;
			glm::quat* RotationBuffer;

			EntityManager::instance->GetEntityArr(PositionBuffer, RotationBuffer, Models);

			memcpy(Entity_Position_Updated, PositionBuffer, sizeof(glm::vec3) * MaxEntities);
			memcpy(Entity_Rotation_Updated, RotationBuffer, sizeof(glm::vec4) * MaxEntities);
			memcpy(Entity_Position_Past, PositionBuffer, sizeof(glm::vec3) * MaxEntities);
			memcpy(Entity_Rotation_Past, RotationBuffer, sizeof(glm::vec4) * MaxEntities);
		}

		// Initiate Velocities -- Apply Forces, Apply Current Velocities
		int* ActiveEntities;
		int ActiveEntitiesNO;

		ApplyForcesToActiveEntities(ActiveEntitiesNO, ActiveEntities);

		/// Apply Restraints

		// Prevent Ground Collision
		

		// Find Possible Collisions
		auto PossibleCollisions = SATCollisionDetection(partitionID);

		// Create Collision Queries
		std::vector<SATCollision&> ColliderOnCollider{};
		std::vector<PhysicsCubeTraceInfo> ModelOnX{};
		int i = 0;
		for (auto& PCol : PossibleCollisions)
		{

			if (PhysicsComponents[PCol.A_ID].collderType == ColliderType::Virtual && PhysicsComponents[PCol.B_ID].collderType == ColliderType::Virtual) { // Colliders are both virtual, so only use the SAT collision
				ColliderOnCollider.push_back(PCol);
				continue;
			}

			glm::vec3 rel_velocity = Entity_Velocity[PCol.B_ID] - Entity_Velocity[PCol.A_ID];
			glm::vec3 normal = glm::normalize(rel_velocity);
			glm::quat rotationAtIncidenceA = glm::fastMix(Entity_Rotation_Past[PCol.A_ID], Entity_Rotation_Updated[PCol.A_ID], PCol.timeToTarget / FixedDeltaTime);
			glm::quat rotationAtIncidenceB = glm::fastMix(Entity_Rotation_Past[PCol.B_ID], Entity_Rotation_Updated[PCol.B_ID], PCol.timeToTarget / FixedDeltaTime);

			float A_Size = CalculateShadowCubeSize(normal * rotationAtIncidenceA, PCol.A_ID);
			float B_Size = CalculateShadowCubeSize(normal * rotationAtIncidenceB, PCol.B_ID);

			PhysicsCubeTraceInfo trace{};
			if (A_Size < B_Size)
			{
				trace.positionA = Entity_Position_Past[PCol.A_ID] + Entity_Velocity[PCol.A_ID] * PCol.timeToTarget;
				trace.RotationA = rotationAtIncidenceA;
				trace.ModelTypeA = (int)PhysicsComponents[PCol.A_ID].collderType;

				trace.positionB = Entity_Position_Past[PCol.B_ID] + Entity_Velocity[PCol.B_ID] * PCol.timeToTarget;
				trace.RotationB = rotationAtIncidenceB;
				trace.ModelTypeB = (int)PhysicsComponents[PCol.B_ID].collderType;
			}
			else
			{

				// Prefer rays come from smaller target
				trace.positionB = Entity_Position_Past[PCol.A_ID] + Entity_Velocity[PCol.A_ID] * PCol.timeToTarget;
				trace.RotationB = rotationAtIncidenceA;
				trace.ModelTypeB = (int)PhysicsComponents[PCol.A_ID].collderType;

				trace.positionA = Entity_Position_Past[PCol.B_ID] + Entity_Velocity[PCol.B_ID] * PCol.timeToTarget;
				trace.RotationA = rotationAtIncidenceB;
				trace.ModelTypeA = (int)PhysicsComponents[PCol.B_ID].collderType;
			}

			//trace_infos[i].quality = SizeToQuality(size);
			trace.direction = normal;
			trace.MaxDistance = (FixedDeltaTime - PCol.timeToTarget) * glm::dot(rel_velocity,normal);
			ModelOnX.push_back(trace);
			i++;
		}
	

	


		// Apply Initial Phyiscs and Send out collision "feelers"
	

		// Solve Collisions
		
		// Send out inital rays

		/// Resolve Collisions

		// Resolve Virtual Collisions
		for (SATCollision& VCol : ColliderOnCollider)
		{
			ResolvePositionAfterCollision(FixedDeltaTime, VCol.PointOfContact, VCol.MinimumTranslationVector, glm::normalize(VCol.MinimumTranslationVector), PhysicsComponents[VCol.A_ID], VCol.A_ID, PhysicsComponents[VCol.B_ID], VCol.B_ID, Entity_Position_Updated, Entity_Velocity);
		}

		DisjointDispatcher->WaitOnOneFenceMax(&context->fence);

		PhysicsRayCollision* collisions = (PhysicsRayCollision*)CollisionsCallback->GetMemoryCopy(); //Potentially have a int storing the number of collisions

		//for (size_t i = 0; i < ModelOnX.size(); i++)
		//{
			// If the ObjB is closer than the world, collide with it
			//if(collisions[i].distanceToPosition < collisions[i + PossibleCollisions.size()].distanceToPosition)
			//{
			//	if(collisions[i].ObjBID == NoCollision)
		//			continue;
				// Hit ObjB
			//	ResolvePositionAfterCollision(FixedDeltaTime,,,collisions[i].normal,PhysicsComponents[trace_infos[i].ID], trace_infos[i].ID, collisions->objA_position,PhysicsComponents[collisions[i].ObjBID], collisions[i].ObjBID, collisions[i].objB_position, Entity_Position_Updated,Entity_Velocity);
		//	}
		//	else if (collisions[i + PossibleCollisions.size()].ObjBID != NoCollision) {
				// Hit World

		//	if (collisions[i].ObjBID == NoCollision)
		//					continue;


		//	}

		//}
		
		// Find World Collisions
		int temp;
		Shaders::Shaders[Shaders::ObjectWorldCollisionDetection]->dispatch(context, MaxEntities, 1, 1, sizeof(int), &temp, MonoidList(4).bind(EntityManager::instance->EntityVec3Data)->bind(EntityManager::instance->EntityVec4Data)->bind(Models)->bind(ModelManager::instance->modelBuffer)->render(), false);

		for (size_t i = 0; i < ActiveEntitiesNO; i++)
		{
			int j = ActiveEntities[i];
			if (collisions[j].ObjBID == NoCollision)
					continue;

			ResolvePositionAfterCollision_World(collisions[j], PhysicsComponents[j],j,Entity_Position_Updated, Entity_Velocity);
		}

	
		// INTEGRATE --- Update Angular Velocity and Velocity 
		for (size_t i = 0; i < ActiveEntitiesNO; i++)
		{
			UpdateVelocity(FixedDeltaTime, ActiveEntities[i], Entity_Velocity, Entity_Position_Updated, Entity_Position_Past);
			UpdateAngularVelocity(FixedDeltaTime, ActiveEntities[i], Entity_Angular_Velocity, Entity_Rotation_Updated, Entity_Rotation_Past);
		}

		// Resolve Collision Velocities
		
		for (const auto& VCol : ColliderOnCollider)
		{
			ResolveVelocitiesAfterCollision(
				FixedDeltaTime, VCol.ADist, VCol.PointOfContact,glm::normalize(VCol.MinimumTranslationVector),
				Entity_Position_Updated, Entity_Rotation_Updated, Entity_Velocity, Entity_Angular_Velocity,
				Entity_Position_Past, Entity_Rotation_Past, Entity_Velocity_Past, Entity_Angular_Velocity_Past, 
				PhysicsComponents[VCol.A_ID], VCol.A_ID, PhysicsComponents[VCol.B_ID], VCol.B_ID);

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
				Entity_Position_Updated, Entity_Rotation_Updated, Entity_Velocity, Entity_Angular_Velocity,
				Entity_Position_Past, Entity_Rotation_Past, Entity_Velocity_Past, Entity_Angular_Velocity_Past,
				PhysicsComponents[j], j, PhysicsComponents[collision.ObjBID], collision.ObjBID);
		}

		// Check Activity of Entities

		for (size_t i = 0; i < ActiveEntitiesNO; i++)
		{
			inactive_time[ActiveEntities[i]] += int(glm::dot(Entity_Velocity[ActiveEntities[i]], Entity_Velocity[ActiveEntities[i]]) < minVelocity) * FixedDeltaTime;

			int mask = 0;
			int bump = (ActiveEntities[i] & (ENTITYCHUNK_SIZE - 1));
			mask |= 1 << bump;

			int entityChunk = ActiveEntities[i] / ENTITYCHUNK_SIZE;
			active_masks[entityChunk] = (active_masks[entityChunk] & ~mask) | ((inactive_time[ActiveEntities[i]] > IDLETIME)<<bump);
		}


		// Wipe Collisions data

		free(ActiveEntities);
	}

public:
	inline static PhysicsManager* Physics{};

	ThreadStatus Status_Get() {
		status_mutex.lock();
		ThreadStatus stat = status;
		status_mutex.unlock();

		return stat;
	}

	void Status_Set(ThreadStatus status) {
		status_mutex.lock();
		this->status = status;
		status_mutex.unlock();
	}

	PhysicsManager() {

		if (Physics != nullptr)
			throw new std::exception("you cannot have more than one physics managers running at a time");

		Physics = this;
		context = new nve::ProductionPackage(QueueType::Graphics);

		PhysicsComponents = (PhysicsComponent*)malloc(sizeof(PhysicsComponent) * MaxEntities);
		Entity_Position_Updated = (glm::vec3*)malloc(sizeof(glm::vec3) * MaxEntities);
		Entity_Rotation_Updated = (glm::quat*)malloc(sizeof(glm::quat) * MaxEntities);
		Entity_Angular_Velocity_Past = (glm::quat*)malloc(sizeof(glm::quat) * MaxEntities);
		Entity_Angular_Velocity = (glm::quat*)malloc(sizeof(glm::quat) * MaxEntities);
		Entity_Velocity_Past = (glm::vec3*)malloc(sizeof(glm::vec3) * MaxEntities);
		Entity_Velocity = (glm::vec3*)malloc(sizeof(glm::vec3) * MaxEntities);
		inactive_time = (float*)malloc(sizeof(float) * MaxEntities);



		auto info = ComputeBufferInfo(sizeof(PhysicsCubeTraceInfo), MaxEntities);
	//	info.properties |= CallbackGPUMemory::IdealPropertyFlags;
		Collision_Information = new ComputeBuffer(info);
		
		auto info_collision = ComputeBufferInfo(sizeof(PhysicsRayCollision), MaxEntities*MaxRaysPerFace);
		auto info_collision_work = ComputeBufferInfo(sizeof(PhysicsRayCollision_Work), MaxEntities*MaxRaysPerFace);

		// Create buffers to work on collisions
		info.properties |= CallbackGPUMemory::IdealPropertyFlags;
		Collision_Information = new ComputeBuffer(info_collision);

		CollisionsCallback = new CallbackGPUMemory(&Collision_Information->memory, Collision_Information->info.length);

	}

	void InitiateNewPhysicsUpdate(bool WaitTilLastPhysicsEnded) {
		
		while (WaitTilLastPhysicsEnded)
		{
			if (Status_Get() == ThreadStatus::Ready)
				break;
		}

		newPhysicsFrame = true;

	}

	void AddPhysicsObject(Entity* entity){
		const glm::vec3 pos = entity->Get_Position();
		int partitionID = GetPartition(pos);
		partitions[partitionID].AddEntity(entity, pos);
	}

	void RemovePhysicsObject(Entity* entity) {
		const glm::vec3 pos = entity->Get_Position();
		int partitionID = GetPartition(entity->Get_Position());
		partitions[partitionID].RemoveEntity(entity, pos);
	}
};