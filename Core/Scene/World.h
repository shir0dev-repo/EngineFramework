#pragma once

#include "Core/Scene/SceneNode.h"
#include "shml/vec3f.hpp"
#include "Core/Structure/DataPtr.h"

class EntityCommandBuffer;

template <typename T>
struct linkedList;

class World {
	EntityCommandBuffer* commandBuffer = nullptr;
	SceneNode* rootNode = nullptr;
	linkedList<DataPtr>* entityCommandData = nullptr;

	static void moveEntityCommand(World* world, Entity* entity);
public:
	World();
	~World();

	SceneNode* const getRootNode();
	
	void moveEntity(Entity* entity, const shml::vec3f& position);
	void rotateEntityCommand(Entity* entity, const shml::vec3f& rotationEuler);

	void addChildCommand(Entity* parent, Entity* child);

	void executeCommands();
};