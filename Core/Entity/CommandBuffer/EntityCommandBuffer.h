#pragma once

struct Entity;

typedef void (*EntityCommandDelegate)(Entity* entity);
template <typename T>
struct linkedList;

class EntityCommandBuffer {
	struct CommandEntry {
		Entity* entity = nullptr;
		EntityCommandDelegate command = nullptr;
	};

	linkedList<CommandEntry>* commandList = nullptr;

public:
	EntityCommandBuffer();
	void addCommand(Entity* entity, EntityCommandDelegate action);
	void execute();

	void clear();
	void dispose();
};