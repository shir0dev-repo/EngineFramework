#version 450

#extension GL_KHR_vulkan_glsl : enable

layout (set = 2, binding = 0) uniform sampler2D top;
layout (set = 2, binding = 1) uniform sampler2D front;
layout (set = 2, binding = 2) uniform sampler2D right;
layout (set = 2, binding = 3) uniform sampler2D back;
layout (set = 2, binding = 4) uniform sampler2D left;
layout (set = 2, binding = 5) uniform sampler2D bottom;

vec3 vertices[8] = {
	vec3(0.5,  -0.5, -0.5), // 0
	vec3(-0.5, -0.5, -0.5), // 1
	vec3(-0.5, -0.5, 0.5),	// 2
	vec3(0.5,  -0.5, 0.5),	// 3

	vec3(0.5,  0.5, -0.5),	// 4
	vec3(-0.5, 0.5, -0.5),	// 5
	vec3(-0.5, 0.5, 0.5),	// 6
	vec3(0.5,  0.5, 0.5),	// 7
};
vec3 centers[6] = {
	vec3(0, 1, 0),	// top
	vec3(0, 0, -1), // front
	vec3(1, 0, 0),	// right
	vec3(0, 0, 1),	// back
	vec3(-1, 0, 0),	// left
	vec3(0, -1, 0)	// bottom
};

int getClosestSampler(in vec3 pos) {
	float dotOutput[6] = {
		dot(pos, centers[0]),
		dot(pos, centers[1]),
		dot(pos, centers[2]),
		dot(pos, centers[3]),
		dot(pos, centers[4]),
		dot(pos, centers[5])
	};

	float highest = pow(10, -45);
	int highestIndex = 0;
	for (int i = 0; i < 6; i++) {
		if (dotOutput[i] > highest) {
			highest = dotOutput[i];
			highestIndex = i;
		}
	}

	return highestIndex;
}

vec4 sampleAs3DTexture(in vec3 screenPosition) {
	vec3 pos = normalize(screenPosition);
	vec4 result;
	
	int highestIndex = getClosestSampler(pos);
	switch (highestIndex) {
		case 0:
			result = texture(top, pos.xy);
			break;
		case 1:
			result = texture(front, pos.xy);
			break;
		case 2:
			result = texture(right, pos.xy);
			break;
		case 3:
			result = texture(back, pos.xy);
			break;
		case 4:
			result = texture(left, pos.xy);
			break;
		default:
			result = texture(bottom, pos.xy);
			break;
	}

	return result;
}

layout (location = 0) in vec4 inPosition;

layout (location = 0) out vec4 outColor;

void main() {
	outColor = sampleAs3DTexture(inPosition.xyz);
}