#include "Core/Scene/NodeMover.h"
#include "Core/Scene/World.h"

void NodeMover::update(World* world, const float dt) {
	if (target) {
		world->moveEntity(this, getLocalPosition() + velocity);
	}
}