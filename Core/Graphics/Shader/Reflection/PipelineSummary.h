#pragma once

typedef unsigned int uint32_t;

struct VkDescriptorSetLayout_T;

struct ShaderModule;
struct DescriptorBindingInfo;
struct PushConstantInfo;
struct VertexAttributeInfo;
struct FragmentOutputInfo;

struct PipelineSummary {
	uint32_t numDescriptorBindingInfos = 0;
	uint32_t numPushConstantInfos = 0;
	uint32_t numVertexInputAttributeInfos = 0;
	uint32_t numFragmentOutputInfos = 0;

	DescriptorBindingInfo* pDescriptorBindingInfos = nullptr;
	PushConstantInfo* pPushConstantInfos = nullptr;
	VertexAttributeInfo* pVertexInputAttributeInfos = nullptr;
	FragmentOutputInfo* pFragmentOutputInfos = nullptr;

	static PipelineSummary* createSummary(ShaderModule** const stages, uint32_t numStages);
};