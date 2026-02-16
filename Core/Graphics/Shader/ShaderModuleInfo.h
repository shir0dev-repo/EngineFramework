#pragma once

typedef unsigned int uint32_t;
typedef enum VkShaderStageFlagBits;

struct SpvReflectShaderModule;
struct DescriptorBindingInfo;
struct PushConstantInfo;
struct VertexAttributeInfo;
struct FragmentOutputInfo;
struct SpecializationConstantInfo;

struct ShaderModuleInfo {
	VkShaderStageFlagBits stage;
	char entry[28] = {0};

	struct GlobalInfo {
		uint32_t count = 0;
		DescriptorBindingInfo* pDescriptorBindingInfos = nullptr;
	} globalDescriptors;

	struct PipelineInfo {
		uint32_t count = 0;
		DescriptorBindingInfo* pDescriptorBindingInfos = nullptr;
	} pipelineDescriptors;

	struct MaterialInfo {
		uint32_t count = 0;
		DescriptorBindingInfo* pDescriptorBindingInfos = nullptr;
	} materialDescriptors;

	uint32_t numPushConstantInfos = 0;
	uint32_t numVertexInputAttributeInfos = 0;
	uint32_t numFragmentOutputInfos = 0;
	uint32_t numSpecializationConstantInfos = 0;

	PushConstantInfo* pPushConstantInfos = nullptr;
	VertexAttributeInfo* pVertexInputAttributeInfos = nullptr;
	FragmentOutputInfo* pFragmentOutputInfos = nullptr;
	SpecializationConstantInfo* pSpecializationConstantInfos = nullptr;

	static ShaderModuleInfo createModuleInfo(const char* shaderCode, const uint32_t& codeLen);
	static void reflectDescriptorBindings(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo);
	static void reflectPushConstants(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo);
	static void reflectVertexInputAttributeInfo(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo);
	static void reflectFragmentOutputInfo(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo);
	static void reflectSpecializationConstantInfo(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo);
};