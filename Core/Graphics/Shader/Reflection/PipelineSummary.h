#pragma once

typedef unsigned int uint32_t;

struct VkDescriptorSetLayout_T;

struct ShaderModule;
struct DescriptorBindingInfo;
struct PushConstantInfo;
struct VertexAttributeInfo;
struct FragmentOutputInfo;

struct PipelineSummary {
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

	struct InstanceInfo {
		uint32_t count = 0;
		DescriptorBindingInfo* pDescriptorBindingInfos = nullptr;
	} instanceDescriptors;

	uint32_t numPushConstantInfos = 0;
	uint32_t numVertexInputAttributeInfos = 0;
	uint32_t numFragmentOutputInfos = 0;

	PushConstantInfo* pPushConstantInfos = nullptr;
	VertexAttributeInfo* pVertexInputAttributeInfos = nullptr;
	FragmentOutputInfo* pFragmentOutputInfos = nullptr;

	static PipelineSummary* createSummary(ShaderModule** const stages, uint32_t numStages);
};