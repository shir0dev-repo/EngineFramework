#include "../Entity.h"

using vec3f = shml::vec3f;
using quat = shml::quat;

vec3f Entity::getLocalPosition() const {
	return this->transform.getPosition();
}

void Entity::setPosition(vec3f position) {
	this->transform.setPosition(position);
}

quat Entity::getRotation() const {
	return this->transform.getRotation();
}

void Entity::setRotation(quat rotation) {
	this->transform.setRotation(rotation);
}

shml::matrix4f Entity::getTransform() const {
	return this->transform;
}