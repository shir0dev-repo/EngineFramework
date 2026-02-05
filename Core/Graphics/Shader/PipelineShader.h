#pragma once

struct ShaderModule;

struct PipelineShader {
	ShaderModule* vertexModule = nullptr;
	ShaderModule* fragmentModule = nullptr;
};