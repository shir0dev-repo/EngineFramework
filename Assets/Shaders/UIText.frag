#version 450
#extension GL_KHR_vulkan_glsl : enable

layout (location = 0) in vec2 inUV;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec4 inColor;

layout (set = 2, binding = 0) uniform sampler2D fontAtlas;

layout (location = 0) out vec4 outColor;

void main() {
	float fontCol = texture(fontAtlas, inUV).r;
	outColor = vec4(clamp(fontCol * inColor.xyz, 0, 1), inColor.a);
}