#pragma once

#include "Core/Entity/Entity.h"

class IEntityComponent {
protected:
	Entity* entity = nullptr;

	IEntityComponent(Entity* const entity) {
		this->entity = entity;
	}

public:
	Entity* const getEntity() { return entity; }
};