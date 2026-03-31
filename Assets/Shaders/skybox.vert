#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 0) uniform MatrixBufferObject {
mat4x4 view;
mat4x4 proj;
vec4 projectionParams;
} Matrices;

layout(set = 3, binding = 0) uniform TransformData {
	mat4x4 localToWorldMatrix;
} transformData;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec4 outPosition;

void main() {
	outPosition = Matrices.proj * Matrices.view * transformData.localToWorldMatrix * inPosition;
	gl_Position = outPosition;
}