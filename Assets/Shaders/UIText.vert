#version 450

layout(set = 0, binding = 0) uniform MatrixBufferObject {
mat4x4 view;
mat4x4 proj;
vec4 projectionParams;
} Matrices;

layout (location = 0) in vec4 screenPosition;
layout (location = 1) in vec4 uv;
layout (location = 2) in vec4 normal;
layout (location = 3) in vec4 inColor;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec4 outColor;

void main() {
	gl_Position = vec4(screenPosition.xyz, 1.0);
	outUV = uv.xy;
	outNormal = normal.xyz;
	outColor = uv.xyxy;
}