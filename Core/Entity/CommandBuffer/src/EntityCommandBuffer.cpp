#include "Core/Entity/CommandBuffer/EntityCommandBuffer.h"

#include "Core/Entity/Entity.h"
#include "Core/Structure/linkedList.h"
#include "Core/Scene/World.h"

EntityCommandBuffer::EntityCommandBuffer() {
	this->commandList = new linkedList<CommandEntry>();
}

void EntityCommandBuffer::addCommand(Entity* entity, EntityCommandDelegate action) {
	commandList->add({entity, action});
}

void EntityCommandBuffer::execute(World* world) {
	for (int i = 0; i < commandList->size(); i++) {
		CommandEntry entry = (*commandList)[i];
		
		entry.command(world, entry.entity);
	}
}

void EntityCommandBuffer::clear() {
	commandList->clear();
}

void EntityCommandBuffer::dispose() {
	commandList->clear();
	delete commandList;
}