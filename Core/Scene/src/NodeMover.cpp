#include "Core/Scene/NodeMover.h"
#include "Core/Scene/World.h"

void NodeMover::update(World* world, const float dt) {
	if (target && velocity != shml::vec3f::ZERO) {
		world->moveEntity(target, getWorldPosition() + velocity * speed);
	}
}