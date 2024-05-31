#pragma once
#include "Physics_Component.h"
#include "EntityConst.h"
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

#include "Physics_Collisions.h"
class PhysicsManager {
	PhysicsComponent* PhysicsComponents{};
	Entity* PhysicsEntities{};

	std::jthread thread;
	ThreadStatus status = ThreadStatus::Starting;
	std::mutex status_mutex{};

	///////// For SIMD

	uint32_t* active_masks{};

	glm::vec3* Entity_Position_Past;
	glm::quat* Entity_Rotation_Past;
	glm::vec3* Entity_Position_Updated;
	glm::quat* Entity_Rotation_Updated;
	glm::quat* Entity_Angular_Velocity;
	glm::quat* Entity_Angular_Velocity_Past;
	glm::vec3* Entity_Velocity;
	glm::vec3* Entity_Velocity_Past;
	//	glm::vec3* Collider_Position_WorldSpace;
	glm::vec3* Collider_Dimension_WorldSpace;

	///////
	//	glm::vec3* WorldPositionsToWork;
	//	glm::vec3* WorldDimensionsToWork;
	//////// Collisions

	ComputeBuffer* Collision_Information;
	ComputeBuffer* RayCollisions_WorkSpace;
	ComputeBuffer* RayCollisions;
	CallbackGPUMemory* CollisionsCallback;
	////////
	
	const int ENTITYCHUNK_SIZE = sizeof(int);

		// Map in place until we add in space partitioning
	PhysicsPartition partitions[9];
	bool newPhysicsFrame = false;

	int GetPartition(glm::vec3 position) {
		return 0;
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

	std::vector<SATCollision> SATCollisionDetection(int partitionID) {

		std::vector<SATCollision> collisions;
		auto partition = partitions[partitionID];
		int i = 0;
		for (auto it = partitions[partitionID].EntityMap.begin(); it != partition.EntityMap.end(); it++, i++)
		{
			auto jt = partitions[partitionID].EntityMap.begin();
			for (size_t j = 0; j <= i; j++)
				jt++; // step up to the current position to ensure a strict order

			for (; jt != partition.EntityMap.end(); jt++)
			{
				auto iobj = it->first;
				auto jobj = jt->first;

				if (!(isEntityActive(iobj) || isEntityActive(jobj))) // if either are active then allow the collision. Else neither are moving, skip!
					continue;


				SATCollision collision{};

				if (SATOptimised(Entity_Position_Updated, Entity_Rotation_Updated, iobj, PhysicsComponents[iobj].dimensions, jobj, PhysicsComponents[jobj].dimensions, collision, Entity_Velocity[iobj] - Entity_Velocity[jobj])) {

					collision.A_ID = iobj;
					collision.B_ID = jobj;

					collisions.push_back(collision);
				}
			}
		}

	}


	void ApplyForcesToActiveEntities()
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
		PhysicsPartition* partition = &partitions[partitionID];
		{
			glm::vec3* PositionBuffer;
			glm::quat* RotationBuffer;

			EntityManager::instance->GetEntityArr(PositionBuffer, RotationBuffer);

			memcpy(Entity_Position_Updated, PositionBuffer, sizeof(glm::vec3) * MaxEntities);
			memcpy(Entity_Rotation_Updated, RotationBuffer, sizeof(glm::vec4) * MaxEntities);
			memcpy(Entity_Position_Past, PositionBuffer, sizeof(glm::vec3) * MaxEntities);
			memcpy(Entity_Rotation_Past, RotationBuffer, sizeof(glm::vec4) * MaxEntities);
		}

		// Initiate Velocities -- Apply Forces, Apply Current Velocities
		ApplyForcesToActiveEntities();

		/// Apply Restraints



		// Find Possible Collisions
		auto PossibleCollisions = SATCollisionDetection(partitionID);

		// Create Collision Queries
		std::vector<const SATCollision&> ColliderOnCollider{};
		std::vector<PhysicsCubeTraceInfo> ModelOnX{};
		int i = 0;
		for (const auto& PCol : PossibleCollisions)
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

		// Pair up Collisions via total ordering of ID

		// Refire remaning unpaired collisions
		
		
		// Resolve Collisions

		PhysicsRayCollision* collisions = (PhysicsRayCollision*)CollisionsCallback->GetMemoryCopy(); //Potentially have a int storing the number of collisions
		for (size_t i = 0; i < NoActiveEntities; i++)
		{
			ResolvePositionAfterCollision(FixedDeltaTime,,,collisions[i].normal,PhysicsComponents[trace_infos[i].ID], trace_infos[i].ID, collisions->objA_position,PhysicsComponents[collisions[i].ObjBID], collisions[i].ObjBID, collisions[i].objB_position, Entity_Position_Updated,Entity_Velocity);
		}
	
		
		for (size_t i = 0; i < NoActiveEntities; i++)
		{
			UpdateVelocity(FixedDeltaTime, trace_infos[i].ID, Entity_Velocity, Entity_Position_Updated, Entity_Position_Past);
			UpdateAngularVelocity(FixedDeltaTime, trace_infos[i].ID, Entity_Angular_Velocity, Entity_Rotation_Updated, Entity_Rotation_Past);
		}
		
		for (size_t i = 0; i < NoActiveEntities; i++)
		{
			PhysicsRayCollision collision = collisions[trace_infos[i].ID];

			if (collision.ObjBID == NoCollision)
				continue;

			ResolveCollision(trace_infos[i].ID, collision.objA_position, collision.ObjBID, collision.objB_position, collision.distanceToPosition);
		}


		// INTEGRATE --- Update Angular Velocity and Velocity 


		// Wipe Collisions data

		free(trace_infos);
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

		PhysicsComponents = (PhysicsComponent*)malloc(sizeof(PhysicsComponent) * MaxEntities);
		Entity_Position_Updated = (glm::vec3*)malloc(sizeof(glm::vec3) * MaxEntities);
		Entity_Rotation_Updated = (glm::quat*)malloc(sizeof(glm::quat) * MaxEntities);
		Entity_Angular_Velocity_Past = (glm::quat*)malloc(sizeof(glm::quat) * MaxEntities);
		Entity_Angular_Velocity = (glm::quat*)malloc(sizeof(glm::quat) * MaxEntities);
		Entity_Velocity_Past = (glm::vec3*)malloc(sizeof(glm::vec3) * MaxEntities);
		Entity_Velocity = (glm::vec3*)malloc(sizeof(glm::vec3) * MaxEntities);



		auto info = ComputeBufferInfo(sizeof(PhysicsCubeTraceInfo), MaxEntities);
	//	info.properties |= CallbackGPUMemory::IdealPropertyFlags;
		Collision_Information = new ComputeBuffer(info);
		
		auto info_collision = ComputeBufferInfo(sizeof(PhysicsRayCollision), MaxEntities*MaxRaysPerFace);
		auto info_collision_work = ComputeBufferInfo(sizeof(PhysicsRayCollision_Work), MaxEntities*MaxRaysPerFace);

		// Create buffers to work on collisions
		RayCollisions_WorkSpace = new ComputeBuffer(info_collision_work);

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
		int partitionID = GetPartition(entity->Get_Position());
		partitions[partitionID].AddEntity(entity);
	}

	void RemovePhysicsObject(Entity* entity) {
		int partitionID = GetPartition(entity->Get_Position());
		partitions[partitionID].RemoveEntity(entity->ID_Get());
	}
};