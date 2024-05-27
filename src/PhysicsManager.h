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
#include "Physics_CubeTraceInfo.h"
#include "../shaders/ChunkGPUConst.h"
#include "EntityManager.h"
#include "Physics_Const.h"
#include "Physics_RayCollision.h"
class PhysicsManager {
	PhysicsComponent* PhysicsComponents{};
	Entity* PhysicsEntities{};

	std::jthread thread;
	ThreadStatus status = ThreadStatus::Starting;
	std::mutex status_mutex{};

///////// For SIMD
	
	uint32_t* active_masks{};

	glm::vec3* Entity_Position_WorldSpace;

//	glm::vec3* Collider_Position_WorldSpace;
	glm::vec3* Collider_Dimension_WorldSpace;

///////
//	glm::vec3* WorldPositionsToWork;
//	glm::vec3* WorldDimensionsToWork;
////////

	ComputeBuffer* Collision_Information;
	ComputeBuffer* RayCollisions_WorkSpace;
	ComputeBuffer* RayCollisions;
	CallbackGPUMemory* CollisionsCallback;
////////

	// Map in place until we add in space partitioning
	PhysicsPartition partitions[6];
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

	float CalculateShadowCubeSize(const glm::vec3 direction_rotated_to_obj_space, glm::vec3 dimensions) {

		dimensions /= glm::vec3(2);

		float lowHigh_X[2] = {-dimensions.x, dimensions.x };
		float lowHigh_Y[2] = {-dimensions.y, dimensions.y };
		float lowHigh_Z[2] = {-dimensions.z, dimensions.z };


		glm::vec2 dot_x;
		glm::vec2 dot_y;
		glm::vec2 dot_z;

		dot_x.x = direction_rotated_to_obj_space.x * lowHigh_X[0];
		dot_x.y = -dot_x.x;

		dot_y.x = direction_rotated_to_obj_space.y * lowHigh_Y[0];
		dot_y.y = -dot_y.x;
		
		dot_z.x = direction_rotated_to_obj_space.z * lowHigh_Z[0];
		dot_z.y = -dot_z.x;

		glm::vec3 furthestCorners{};
		furthestCorners.x = (std::abs(dot_x.x) < std::abs(dot_x.y)) ? lowHigh_X[0] : lowHigh_X[1];
		furthestCorners.y = (std::abs(dot_y.x) < std::abs(dot_y.y)) ? lowHigh_Y[0] : lowHigh_Y[1];
		furthestCorners.z = (std::abs(dot_z.x) < std::abs(dot_z.y)) ? lowHigh_Z[0] : lowHigh_Z[1];

		return glm::distance(furthestCorners, -furthestCorners);
	}

	void UpdateEntityChunk(int i) {
		
		for (size_t j = 0; j < 32; j++)
		{
			//Collider_Position_WorldSpace [i + j] = PhysicsEntities[j + i].Get_Position();
			Collider_Dimension_WorldSpace[i + j] = PhysicsEntities[j + i].Get_Rotation() + PhysicsComponents[j + i].dimension;
		}
		
	/*	for (size_t j = 0; j < 32; j++)
		{
			WorldPositionsToWork[j] = PhysicsEntities[j + i].Get_Position() + PhysicsComponents[j + i].origin;
			WorldDimensionsToWork[j] = PhysicsEntities[j + i].Get_Rotation() + PhysicsComponents[j + i].dimension;
		}*/
	}

	void ApplyGravity(int ID) {
		Entity_Position_WorldSpace[ID] += PhysicsComponents[ID].Gravity * FixedDeltaTime;
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



	void PhysicsUpdate() {

		PhysicsPartition* partition = &partitions[0];

		EntityGPUStruct* GPUData = EntityManager::instance->GetEntityArr();
		memcpy(Entity_Position_WorldSpace, GPUData, sizeof(EntityGPUStruct) * MaxEntities);
		

		std::vector<int> ActiveChunks;
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
	
	
		// Apply Initial Phyiscs and Send out collision "feelers"

		PhysicsCubeTraceInfo* trace_infos = (PhysicsCubeTraceInfo*)malloc(sizeof(PhysicsCubeTraceInfo) * NoActiveEntities);
		for (size_t i = 0; i < ActiveChunks.size(); i++)
		{

			uint32_t worked_mask = active_masks[ActiveChunks[i]];
			for (size_t k = 0; k < 32; k++)
			{
				
				int EntityOffset = worked_mask & ~(worked_mask - 1); // Get lowest bit
				worked_mask -= EntityOffset; //Remove it from temporary mask

				int ID = ActiveChunks[i] * sizeof(worked_mask)*8 + EntityOffset; 


				// Apply Gravity
				ApplyGravity(ID);


				glm::vec3 normal = glm::normalize(PhysicsComponents[ID].Velocity);

				float size = CalculateShadowCubeSize(normal, PhysicsComponents->dimension);

				trace_infos[i].quality = SizeToQuality(size);
				trace_infos[i].position = Entity_Position_WorldSpace[ID];
				trace_infos[i].direction = normal;
				trace_infos[i].ID = ID;

				if (worked_mask == 0) // if empty skip the rest of the chunk
					break;
			}

		}

		// Solve Collisions
		
		// Send out inital rays

		// Pair up Collisions via total ordering of ID

		// Refire remaning unpaired collisions


		PhysicsRayCollision* collisions = (PhysicsRayCollision*)CollisionsCallback->GetMemoryCopy(); //Potentially have a int storing the number of collisions
		for (size_t i = 0; i < NoActiveEntities; i++)
		{
			PhysicsRayCollision collision = collisions[trace_infos[i].ID];

			if (collision.ObjBID == NoCollision)
				continue;

			ResolveCollision(trace_infos[i].ID, collision.objA_position, collision.ObjBID, collision.objB_position, collision.distanceToPosition);
		}


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