#pragma once

#include <stdint.h>
#include "mathutil.hpp"

namespace shml {
	typedef struct vec3i {
		static vec3i ZERO;
		static vec3i LEFT;
		static vec3i RIGHT;
		static vec3i DOWN;
		static vec3i UP;
		static vec3i BACK;
		static vec3i FORWARD;
		static vec3i ONE;

		int32_t x, y, z;

		inline vec3i() :x(0), y(0), z(0) {}

		inline vec3i(const int32_t& x, const int32_t& y, const int32_t& z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		inline vec3i(const vec3i& v) {
			*this = v;
		}

		inline vec3i operator=(const vec3i& v) {
			set(v.x, v.y, v.z);
		}

		inline void set(int32_t x, int32_t y, int32_t z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		inline vec3i normalized() const {
			vec3i v;
			v.x = sign(x);
			v.y = sign(y);
			v.z = sign(z);

			return v;
		}

		inline void normalize() {
			x = sign(x);
			y = sign(y);
			z = sign(z);
		}

		inline float length() const {
			return sqrtf(lengthSquared());
		}

		inline int32_t lengthSquared() const {
			return x * x + y * y + z * z;
		}

		inline static float dot(const vec3i& a, const vec3i& b) {
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		inline static vec3i cross(const vec3i& a, const vec3i& b) {
			vec3i c;
			c.x = (a.y * b.z) - (a.z * b.y);
			c.y = (a.z * b.x) - (a.x * b.z);
			c.z = (a.x * b.y) - (a.y * b.x);

			return c;
		}

		inline vec3i operator-(const vec3i& v) const {
			vec3i v1(*this);
			v1.x -= v.x;
			v1.y -= v.y;
			v1.z -= v.z;

			return v1;
		}

		inline vec3i& operator-=(const vec3i& v) {
			x -= v.x;
			y -= v.y;
			z -= v.z;

			return *this;
		}

		inline vec3i operator*(const float& scalar) const {
			vec3i v(*this);

			v.x *= scalar;
			v.y *= scalar;
			v.z *= scalar;

			return v;
		}

		inline vec3i& operator*=(const float& scalar) {
			x *= scalar;
			y *= scalar;
			z *= scalar;

			return *this;
		}

		inline vec3i operator*(const vec3i& v) const {
			vec3i v1(*this);

			v1.x *= v.x;
			v1.y *= v.y;
			v1.z *= v.z;

			return v1;
		}

		inline vec3i& operator*=(const vec3i& v) {
			x *= v.x;
			y *= v.y;
			z *= v.z;

			return *this;
		}

		inline vec3i operator-() const {
			vec3i v(*this);
			v.x *= -1;
			v.y *= -1;
			v.z *= -1;

			return v;
		}

		inline bool operator==(const vec3i& other) const {
			return x == other.x && y == other.y && z == other.z;
		}

		inline bool operator!=(const vec3i& other) const {
			return !(*this == other);
		}

		inline operator vec3f() const {
			vec3f v;
			v.x = (float)x;
			v.y = (float)y;
			v.z = (float)z;

			return v;
		}

		inline friend std::ostream& operator<<(std::ostream& os, const vec3i& v) {
			os << std::fixed << std::setprecision(3) <<
				"[V3]: " << '(' <<
				"X: " << v.x << ", " <<
				"Y: " << v.y << ", " <<
				"Z: " << v.z << ')';

			return os;
		}
	};
}