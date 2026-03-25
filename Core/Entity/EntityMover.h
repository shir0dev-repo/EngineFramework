#pragma once

struct Entity;

#include "shml/quat.hpp"

struct EntityMover {
	static void moveEntity(Entity* entity, shml::vec3f delta);
	static void moveEntityX(Entity* entity, float deltaX);
	static void moveEntityY(Entity* entity, float deltaY);
	static void moveEntityZ(Entity* entity, float deltaZ);

	static void rotateEntity(Entity* entity, const shml::quat delta);
	static void rotateEntityEuler(Entity* entity, const shml::vec3f delta);

	static void rotateEntityX(Entity* entity, const float deltaX);
	static void rotateEntityY(Entity* entity, const float deltaY);
	static void rotateEntityZ(Entity* entity, const float deltaZ);
};