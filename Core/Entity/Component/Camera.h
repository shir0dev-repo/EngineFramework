#pragma once

#include <shml/matrix4f.hpp>

typedef unsigned int uint32_t;

struct Camera {
	shml::matrix4f viewMatrix;
	shml::matrix4f projMatrix;
	shml::vec4f projectionParams;
	shml::matrix4f transform;

	void setup(uint32_t windowWidth, uint32_t windowHeight);
	void updateViewMatrix();
	void regenerateProjectionMatrix(uint32_t windowWidth, uint32_t windowHeight);
};