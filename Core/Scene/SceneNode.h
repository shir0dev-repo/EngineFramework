#pragma once

#include "Core/Entity/Entity.h"
#include "Core/Structure/linkedList.h"

typedef unsigned int uint32_t;

struct SceneNode : public Entity {
	SceneNode* parent = nullptr;
	linkedList<SceneNode*>* children = nullptr;

	SceneNode();
	virtual ~SceneNode();

	virtual void preUpdate(class World* world);
	virtual void onUpdate(class World* world, float dt);
	virtual void update(class World* world, float dt) {}
	virtual void lateUpdate(class World* world, float dt) {}

	virtual void initialize(class World* world) {}
	virtual void onStart(class World* world) {}


	uint32_t uuid();

	void addChild(SceneNode* child);
	bool removeChild(SceneNode* child);

	bool didUpdate = false;
	
};