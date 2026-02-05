#pragma once

#include <shml/matrix4f.hpp>

struct Camera {
	shml::matrix4f transform;
	shml::matrix4f viewMatrix;
	shml::matrix4f projMatrix;
};