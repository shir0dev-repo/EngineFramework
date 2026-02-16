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

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec2 texCoord;

layout(set = 2, binding = 1) uniform MaterialData {
	float specular;
	float metallic;
} materialData;

void main() {
	vec4 position = vec4(inPosition.xyz, 1.0);
	position.xy += materialData.specular;
	gl_Position = Matrices.proj * Matrices.view * position;
	texCoord = uv.xy;
	outColor = vec4(uv.x, uv.y, 0.0, 1.0);
}