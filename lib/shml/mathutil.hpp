#pragma once

#include <algorithm>
#include "vec3f.hpp"

namespace shml {
	const double PI = 3.14159265358979311599796346854;
	const double DEG2RAD = 0.01745f;
	const double RAD2DEG = 57.296f;

	template <typename T>
	inline T clamp(T in, T min, T max) {
		return std::max(min, std::min(in, max));
	}

	inline void clampAngle(float* angle, const bool& radians = false) {
		if (!radians) {
			if (*angle > 180.0)
				*angle -= 360.0;
			else if (*angle < -180.0)
				*angle += 360.0;
		}
		else {
			double twoPI = 2 * PI;

			if (*angle >= twoPI)
				*angle -= twoPI;
			else if (*angle <= 0)
				*angle += twoPI;
		}
	}


	inline void clampAngle(float* angle, const float& minAngle, const double& maxAngle) {
		double temp = *angle;

		if (temp < minAngle) {
			temp = minAngle;
		}
		else if (temp > maxAngle) {
			temp = maxAngle;
		}

		*angle = temp;
	}

	inline void clampEulers(vec3f* eulers, const bool& radians = false) {
		if (!radians) {
			clampAngle(&eulers->y);
			clampAngle(&eulers->x);
			clampAngle(&eulers->z);
		}
		else {
			clampAngle(&eulers->y, true);
			clampAngle(&eulers->x, true);
			clampAngle(&eulers->z, true);
		}
	}

	inline int sign(const int& i) { return (i > 0) - (i < 0); }

	inline float randf(float low, float high) {
		return low + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (high - low)));
	}

	template<typename T>
	inline T nextPowTwo(T num) {
		assert(num > 1);

		uint64_t iterations = sizeof(T) * 8;
		num--;

		for (int i = 1; i < iterations; i <<= 1) {
			num |= num >> i;
		}

		return ++num;
	}
}