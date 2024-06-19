#pragma once
#include "Physics_Component.h"
struct PhysicsComponentCommand {
	PhysicsComponent component;
	bool Destroy;

	PhysicsComponentCommand(PhysicsComponent component, bool destroy = false) : component(component), Destroy(destroy){}
};