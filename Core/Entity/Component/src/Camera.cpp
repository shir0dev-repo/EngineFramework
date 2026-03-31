#include "Core/Entity/Component/Camera.h"

void Camera::setup(uint32_t windowWidth, uint32_t windowHeight) {
	regenerateProjectionMatrix(windowWidth, windowHeight);
	transform.setPosition({0, 0, -5});
	updateViewMatrix();
}

void Camera::updateViewMatrix() {
	viewMatrix = transform.inverted();
}

void Camera::regenerateProjectionMatrix(uint32_t windowWidth, uint32_t windowHeight) {
	projMatrix = shml::matrix4f::IDENTITY;
	const float nearPlane = 0.03f, farPlane = 100.0f;

	float aspect = windowWidth / windowHeight;
	float fov = 60.0f;
	float scale = tan(fov * 0.5f * shml::DEG2RAD);
	projMatrix(0, 0) = 1.0f / (aspect * scale);
	projMatrix(1, 1) = -1.0f / scale;
	projMatrix(2, 2) = -(farPlane / (farPlane - nearPlane));
	projMatrix(2, 3) = -(2.0 * (farPlane * nearPlane) / (farPlane - nearPlane));
	projMatrix(3, 2) = -1;
	projMatrix(3, 3) = 0;
	projMatrix.transpose();

	projectionParams = { fov, aspect, nearPlane, farPlane };
}