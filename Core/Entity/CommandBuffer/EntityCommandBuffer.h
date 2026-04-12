#pragma once

struct Entity;
struct World;

typedef void (*EntityCommandDelegate)(World* world, Entity* entity);
template <typename T>
class linkedList;

class EntityCommandBuffer {
	struct CommandEntry {
		Entity* entity = nullptr;
		EntityCommandDelegate command = nullptr;
	};

	linkedList<CommandEntry>* commandList = nullptr;

public:
	EntityCommandBuffer();
	void addCommand(Entity* entity, EntityCommandDelegate action);
	void execute(World* world);

	void clear();
	void dispose();
};