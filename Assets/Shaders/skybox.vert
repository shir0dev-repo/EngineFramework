#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(set = 0, binding = 0) uniform MatrixBufferObject {
mat4x4 view;
mat4x4 proj;
vec4 projectionParams;
} Matrices;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec4 outPosition;

void main() {
	vec3 position = inPosition.xyz;// * Matrices.projectionParams.w;
	outPosition = Matrices.proj * Matrices.view * vec4(position, 1.0);
	gl_Position = outPosition;
}