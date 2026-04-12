#pragma once

#include "Core/Scene/SceneNode.h"
#include "Core/Events/KeyboardEvents.h"

#include "GLFW/glfw3.h"

struct NodeMover : public SceneNode {
	shml::vec3f velocity;
	float speed;
	Entity* target;

	void setVelocity(const Event<EKeyboardEvents>& evt) {
		const KeyDownEvent& kb = evt.toType<KeyDownEvent>();

		switch (kb.keyCode) {
		case GLFW_KEY_W:
			velocity.z = -1;
			break;
		case GLFW_KEY_S:
			velocity.z = 1;
			break;
		case GLFW_KEY_A:
			velocity.x = 1;
			break;
		case GLFW_KEY_D:
			velocity.x = -1;
			break;
		case GLFW_KEY_Q:
			velocity.y = -1;
			break;
		case GLFW_KEY_E:
			velocity.y = 1;
			break;
		default:
			break;
		};
	}

	void zeroVelocity(const Event<EKeyboardEvents>& evt) {
		KeyDownEvent kb = evt.toType<KeyDownEvent>();
		switch (kb.keyCode) {
		case GLFW_KEY_W:
			velocity.z = 0;
			break;
		case GLFW_KEY_S:
			velocity.z = 0;
			break;
		case GLFW_KEY_A:
			velocity.x = 0;
			break;
		case GLFW_KEY_D:
			velocity.x = 0;
			break;
		case GLFW_KEY_Q:
			velocity.y = 0;
			break;
		case GLFW_KEY_E:
			velocity.y = 0;
			break;
		};
	}

	NodeMover(Entity* target, SceneNode* parent) : SceneNode() {
		speed = 0.0f;
		this->target = target;
		if (parent) {
			parent->addChild(this);
		}
	}

	virtual void update(class World* world, const float dt) override;
};