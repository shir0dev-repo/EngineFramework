#pragma once

#include <shml/matrix4f.hpp>

struct GPUCameraData {
	shml::matrix4f view = shml::matrix4f::IDENTITY;
	shml::matrix4f projection = shml::matrix4f::IDENTITY;
};