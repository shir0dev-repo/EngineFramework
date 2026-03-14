#include "Core/Scene/World.h"
#include "Core/Entity/CommandBuffer/EntityCommandBuffer.h"
#include "Core/Structure/linkedList.h"

typedef shml::vec3f vec3f;

World::World() {
	this->commandBuffer = new EntityCommandBuffer();
	this->entityCommandData = new linkedList<DataPtr>();
}
World::~World() {
	for (uint32_t i = 0; i < entityCommandData->size(); i++) {
		free((*entityCommandData)[i].Data);
	}

	delete entityCommandData;
	delete commandBuffer;
}

World* const World::getWorld() {
	static World* world;
	if (world == nullptr) {
		world = new World();
		world->rootNode = {};
	}

	return world;
}

SceneNode* const World::getRootNode() {
	return &rootNode;
}

void World::moveEntity(Entity* entity, const shml::vec3f& position) {
	void* data = malloc(sizeof(vec3f));
	if (data != nullptr) {
		memcpy(data, &position, sizeof(vec3f));
	}

	commandBuffer->addCommand(entity, moveEntityCommand);
	entityCommandData->add({ data });
}
void World::moveEntityCommand(Entity* entity) {
	vec3f position = {};
	memcpy(&position, getWorld()->entityCommandData->first(), sizeof(vec3f));

	entity->setPosition(position);
	free((*getWorld()->entityCommandData)[0]);
	getWorld()->entityCommandData->removeAt(0);
}
void World::executeCommands() {
	commandBuffer->execute();
	commandBuffer->clear();
}