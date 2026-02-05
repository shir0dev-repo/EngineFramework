#version 450
#extension GL_KHR_vulkan_glsl : enable

//#include "core.glsl"

layout(set = 0, binding = 0) uniform MatrixBufferObject {
mat4x4 view;
mat4x4 proj;
} Matrices;

layout (location = 0) in vec4 inPosition;
layout (location = 1) in vec4 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 inColor;

layout(location = 0) out vec3 fragColor;

void main() {
	gl_Position = Matrices.proj * Matrices.view * 
	vec4(inPosition.xyz, 1.0);
	fragColor = inColor.xyz;
}