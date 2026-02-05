#pragma once
#include "shml/matrix4f.hpp"

struct MatrixBufferObject {
	shml::matrix4f viewMatrix;
	shml::matrix4f projectionMatrix;
};