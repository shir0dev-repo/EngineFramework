#include "../SceneNode.h"

SceneNode::SceneNode() : Entity() {
	this->children = new linkedList<SceneNode*>();
}

SceneNode::~SceneNode() {
	delete this->children;
	this->children = nullptr;
}

uint32_t SceneNode::uuid() { return reinterpret_cast<uint32_t>(this); }

void SceneNode::preUpdate(World* world) {
	this->didUpdate = false;
	if (children) {
		for (auto& child : *children) {
			child->preUpdate(world);
		}
	}
}

void SceneNode::onUpdate(World* world, float dt) {
	if (didUpdate) {
		return;
	}
	update(world, dt);
	if (children) {
		for (auto& child : *children) {
			child->onUpdate(world, dt);
		}
	}
}

void SceneNode::addChild(SceneNode* child) {
	if (children == nullptr) {
		children = new linkedList<SceneNode*>();
	}

	children->add(child);
}

bool SceneNode::removeChild(SceneNode* child) {
	if (!children) {
		return false;
	}

	int32_t toRemove = -1;
	int32_t current = 0;

	for (auto& it : *children) {
		if (child->uuid()  == it->uuid()) {
			toRemove = current;
			break;
		}
		current++;
	}

	if (toRemove != -1) {
		children->removeAt(toRemove);
	}

	return toRemove != -1;
}