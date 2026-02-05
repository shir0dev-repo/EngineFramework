#pragma once

#include "vec3f.hpp"

namespace shml {
	struct vec4f {
		float x;
		float y;
		float z;
		float w;

		inline vec4f() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) { }

		inline vec4f(const float& x, const float& y, const float& z, const float& w) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}

		inline vec4f(const vec3f& v) {
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
			this->w = 1.0f;
		}

		inline vec4f(const vec4f& v) {
			*this = v;
		}

		inline vec4f& operator=(const vec4f& v) {
			this->x = v.x;
			this->y = v.y;
			this->z = v.z;
			this->w = v.w;

			return *this;
		}

		inline static float dot(const vec4f& a, const vec4f& b) {
			return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
		}

		inline vec4f operator+(const vec4f& v) const {
			vec4f v1(*this);

			v1.x += v.x;
			v1.y += v.y;
			v1.z += v.z;
			v1.w += v.w;

			return v1;
		}

		inline vec4f& operator+=(const vec4f& v) {
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;

			return *this;
		}

		inline vec4f operator-(const vec4f& v) const {
			vec4f v1(*this);
			v1.x -= v.x;
			v1.y -= v.y;
			v1.z -= v.z;
			v1.w -= v.w;

			return v1;
		}

		inline vec4f& operator-=(const vec4f& v) {
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;

			return *this;
		}

		inline bool operator==(const vec4f& other) const {
			return
				x == other.x &&
				y == other.y &&
				z == other.z &&
				w == other.w;
		}

		inline bool operator!=(const vec4f& other) const {
			return !(*this == other);
		}

		// Returns a truncated (x, y, z, no w) version of this vec4f.
		inline explicit operator vec3f() const {
			return vec3f(x, y, z);
		}

		inline explicit operator float*() {
			return &x;
		}
	};
}