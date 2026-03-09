#include "Core/Entity/CommandBuffer/EntityCommandBuffer.h"

#include "Core/Entity/Entity.h"
#include "Core/Structure/linkedList.h"

EntityCommandBuffer::EntityCommandBuffer() {
	this->commandList = new linkedList<CommandEntry>();
}

void EntityCommandBuffer::addCommand(Entity* entity, EntityCommandDelegate action) {
	commandList->add({ entity, action });
}

void EntityCommandBuffer::execute() {
	for (int i = 0; i < commandList->size(); i++) {
		CommandEntry entry = (*commandList)[i];
		entry.command(entry.entity);
	}
}

void EntityCommandBuffer::clear() {
	commandList->clear();
}

void EntityCommandBuffer::dispose() {
	commandList->clear();
	delete commandList;
}