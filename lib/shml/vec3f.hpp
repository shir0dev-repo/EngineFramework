#pragma once

#include <math.h>
#include <cassert>
#include <iomanip>
#include <iostream>

namespace shml {
	struct vec3f {
		static vec3f ZERO;
		static vec3f LEFT;
		static vec3f RIGHT;
		static vec3f DOWN;
		static vec3f UP;
		static vec3f BACK;
		static vec3f FORWARD;
		static vec3f ONE;

		float x;
		float y;
		float z;

		inline vec3f() : x(0.0f), y(0.0f), z(0.0f) { }

		inline vec3f(const float& x, const float& y, const float& z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		inline vec3f(const vec3f& v) {
			*this = v;
		}

		inline vec3f& operator=(const vec3f& v) {
			this->set(v.x, v.y, v.z);

			return *this;
		}

		inline void set(const float& x, const float& y, const float& z) {
			this->x = x;
			this->y = y;
			this->z = z;
		}

		inline float length() const {
			return sqrt(x * x + y * y + z * z);
		}
		inline float lengthSquared() const {
			return x * x + y * y + z * z;
		}

		inline vec3f normalized() const {
			vec3f v(*this);

			float inv_l = 1.0f / v.length();
			v.x *= inv_l;
			v.y *= inv_l;
			v.z *= inv_l;

			return v;
		}

		inline vec3f normalized_safe(float accuracy = 0.003f) const {
			vec3f v(*this);
			float l = length();
			if (abs(l - accuracy) <= accuracy) {
				return v;
			}

			float inv_l = 1.0f / l;
			v.x *= inv_l;
			v.y *= inv_l;
			v.z *= inv_l;
		}

		inline void normalize() {
			float inv_l = 1.0f / length();
			x *= inv_l;
			y *= inv_l;
			z *= inv_l;
		}

		inline void normalize_safe(float accuracy = 0.003f) {
			float l = length();
			if (abs(l - accuracy) <= accuracy) {
				return;
			}

			float inv_l = 1.0f / l;
			x *= inv_l;
			y *= inv_l;
			z *= inv_l;
		}

		inline static float distance(const vec3f& a, const vec3f& b) {
			return sqrtf(distanceSquared(a, b));
		}

		inline static float distanceSquared(const vec3f& a, const vec3f& b) {
			return fabs(powf(b.x - a.x, 2) + powf(b.y - a.y, 2) + powf(b.z - a.z, 2));
		}
		inline static vec3f lerp(const vec3f& a, const vec3f& b, const float& t) {
			return a + (b - a) * t;
		}

		inline static float dot(const vec3f& a, const vec3f& b) {
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}

		inline static vec3f cross(const vec3f& a, const vec3f& b) {
			vec3f c;
			c.x = (a.y * b.z) - (a.z * b.y);
			c.y = (a.z * b.x) - (a.x * b.z);
			c.z = (a.x * b.y) - (a.y * b.x);

			return c;
		}

		inline static vec3f project(const vec3f& in, const vec3f& onto) {
			// scalar projection:
			// vProj = dot(in, onto) * onto
			// ortho projection:
			// vOProj = in - dot(in, onto) * onto

			float d = dot(in, onto);
			float lSqr = onto.lengthSquared();
			return onto * (d / lSqr);
		}

		inline static vec3f orthoProject(const vec3f& in, const vec3f& onto) {
			return in - project(in, onto);
		}

		inline float& operator[](const unsigned int& index) {
			assert(index < 3);
			if (index == 0) return x;
			if (index == 1) return y;
			if (index == 2) return z;
		}


		inline vec3f operator+(const vec3f& v) const {
			vec3f v1(*this);
			v1.x += v.x;
			v1.y += v.y;
			v1.z += v.z;

			return v1;
		}

		inline vec3f& operator+=(const vec3f& v) {
			x += v.x;
			y += v.y;
			z += v.z;

			return *this;
		}

		inline vec3f operator-(const vec3f& v) const {
			vec3f v1(*this);
			v1.x -= v.x;
			v1.y -= v.y;
			v1.z -= v.z;

			return v1;
		}

		inline vec3f& operator-=(const vec3f& v) {
			x -= v.x;
			y -= v.y;
			z -= v.z;

			return *this;
		}

		inline vec3f operator*(const float& scalar) const {
			vec3f v(*this);

			v.x *= scalar;
			v.y *= scalar;
			v.z *= scalar;

			return v;
		}

		inline vec3f& operator*=(const float& scalar) {
			x *= scalar;
			y *= scalar;
			z *= scalar;

			return *this;
		}

		inline vec3f operator*(const vec3f& v) const {
			vec3f v1(*this);

			v1.x *= v.x;
			v1.y *= v.y;
			v1.z *= v.z;

			return v1;
		}

		inline vec3f& operator*=(const vec3f& v) {
			x *= v.x;
			y *= v.y;
			z *= v.z;

			return *this;
		}

		inline vec3f operator-() const {
			vec3f v(*this);
			v.x *= -1;
			v.y *= -1;
			v.z *= -1;

			return v;
		}

		inline bool operator==(const vec3f& other) const {
			return x == other.x && y == other.y && z == other.z;
		}

		inline bool operator!=(const vec3f& other) const {
			return !(*this == other);
		}

		inline explicit operator float* () {
			return &x;
		}

		inline explicit operator const float* () const {
			return &x;
		}

		inline friend std::ostream& operator<<(std::ostream& os, const vec3f& v) {
			os << std::fixed << std::setprecision(3) <<
				"[V3]: " << '(' <<
				"X: " << v.x << ", " <<
				"Y: " << v.y << ", " <<
				"Z: " << v.z << ')';

			return os;
		}
	};
}