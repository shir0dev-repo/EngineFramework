#include "Core/Scene/World.h"
#include "Core/Entity/CommandBuffer/EntityCommandBuffer.h"
#include "Core/Structure/linkedList.h"
#include <functional>

typedef shml::vec3f vec3f;

World::World() {
	this->commandBuffer = new EntityCommandBuffer();
	this->entityCommandData = new linkedList<DataPtr>();
	this->rootNode = new SceneNode();
}
World::~World() {
	for (uint32_t i = 0; i < entityCommandData->size(); i++) {
		free((*entityCommandData)[i].Data);
	}

	delete entityCommandData;
	delete commandBuffer;
}

SceneNode* const World::getRootNode() {
	return rootNode;
}
void World::moveEntityCommand(World* world, Entity* entity) {
	vec3f position = {};
	memcpy(&position, world->entityCommandData->first(), sizeof(vec3f));

	entity->setPosition(position);
	free((*world->entityCommandData)[0]);
	world->entityCommandData->removeAt(0);
}
void World::moveEntity(Entity* entity, const shml::vec3f& position) {
	void* data = malloc(sizeof(vec3f));
	if (data != nullptr) {
		memcpy(data, &position, sizeof(vec3f));
	}

	commandBuffer->addCommand(entity, &World::moveEntityCommand);
	entityCommandData->add({ data });
}



void World::executeCommands() {
	commandBuffer->execute(this);
	commandBuffer->clear();
}