#pragma once

#include <shml/matrix4f.hpp>

struct Entity {
	void setPosition(shml::vec3f localPosition);
	shml::vec3f getLocalPosition() const;

	void setRotation(shml::quat rotation);
	shml::quat getRotation() const;
	
	shml::matrix4f getTransform() const;

private:
	shml::matrix4f transform = shml::matrix4f::IDENTITY;
};