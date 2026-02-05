#pragma once

#include <shml/matrix4f.hpp>

struct GPUCameraData {
	alignas(16) shml::matrix4f view = shml::matrix4f::IDENTITY;
	alignas(16) shml::matrix4f projection = shml::matrix4f::IDENTITY;
};