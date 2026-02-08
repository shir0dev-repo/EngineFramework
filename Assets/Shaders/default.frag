#version 450
#extension GL_KHR_vulkan_glsl : enable

layout(set = 1, binding = 0) uniform sampler2D white;
layout(set = 1, binding = 1) uniform sampler2D normal;

layout(set = 2, binding = 0) uniform sampler2D _mainTexture;
layout(set = 2, binding = 1) uniform MaterialData {
	float specular;
	float metallic;
} materialData;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 texCoord;
layout(location = 0) out vec4 outColor;

void main() {
	vec3 lightDir = vec3(0, 0, -1);

	outColor = materialData.specular * texture(_mainTexture, texCoord);
}