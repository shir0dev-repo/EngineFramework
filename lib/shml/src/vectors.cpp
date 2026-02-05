#include "../vec3f.hpp"
#include "../quat.hpp"
namespace shml {
	quat quat::IDENTITY = quat();

	vec3f vec3f::ZERO = vec3f(0, 0, 0);
	vec3f vec3f::LEFT = vec3f(-1, 0, 0);
	vec3f vec3f::RIGHT = vec3f(1, 0, 0);
	vec3f vec3f::DOWN = vec3f(0, -1, 0);
	vec3f vec3f::UP = vec3f(0, 1, 0);
	vec3f vec3f::BACK = vec3f(0, 0, -1);
	vec3f vec3f::FORWARD = vec3f(0, 0, 1);
	vec3f vec3f::ONE = vec3f(1, 1, 1);
}