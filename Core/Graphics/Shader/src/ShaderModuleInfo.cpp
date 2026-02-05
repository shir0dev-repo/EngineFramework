#include "../ShaderModuleInfo.h"
#include "../Reflection/DescriptorBindingInfo.h"
#include "../Reflection/VertexAttributeInfo.h"
#include "../Reflection/PushConstantInfo.h"
#include "../Reflection/FragmentOutputInfo.h"
#include "../Reflection/SpecializationConstantInfo.h"

#include "SPIRV-Reflect/spirv_reflect.h"
#include <cstring>
#include <iostream>

ShaderModuleInfo ShaderModuleInfo::createModuleInfo(const char* shaderCode, const uint32_t& codeLen) {
	SpvReflectShaderModule reflectModule = {};
	spvReflectCreateShaderModule(codeLen, shaderCode, &reflectModule);

	ShaderModuleInfo moduleInfo = {};
	uint32_t nameLen = strnlen_s(reflectModule.entry_point_name, 17);
	moduleInfo.stage = static_cast<VkShaderStageFlagBits>(reflectModule.shader_stage);
	if (nameLen > 16) {
		throw std::runtime_error("Buffer overrun at shader module entry point name!");
	}
	strcpy_s(&moduleInfo.entry[0], 16, reflectModule.entry_point_name);

	reflectDescriptorBindings(reflectModule, &moduleInfo);
	reflectPushConstants(reflectModule, &moduleInfo);
	if (reflectModule.shader_stage & SPV_REFLECT_SHADER_STAGE_VERTEX_BIT) {
		reflectVertexInputAttributeInfo(reflectModule, &moduleInfo);
	}
	else if (reflectModule.shader_stage & SPV_REFLECT_SHADER_STAGE_FRAGMENT_BIT) {
		reflectFragmentOutputInfo(reflectModule, &moduleInfo);
	}
	reflectSpecializationConstantInfo(reflectModule, &moduleInfo);

	spvReflectDestroyShaderModule(&reflectModule);

	return moduleInfo;
}

void ShaderModuleInfo::reflectDescriptorBindings(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo) {
	uint32_t descriptorSetCount = 0;
	spvReflectEnumerateDescriptorSets(&reflectModule, &descriptorSetCount, nullptr);

	std::vector<SpvReflectDescriptorSet*> descriptorSets(descriptorSetCount);
	spvReflectEnumerateDescriptorSets(&reflectModule, &descriptorSetCount, descriptorSets.data());
	std::vector<DescriptorBindingInfo> bindingInfos{};
	for (auto* set : descriptorSets) {
		for (uint32_t i = 0; i < set->binding_count; i++) {
			const SpvReflectDescriptorBinding* binding = set->bindings[i];

			DescriptorBindingInfo info = {};
			info.setIndex = set->set;
			info.bindingIndex = binding->binding;
			info.count = binding->count;
			info.type = static_cast<VkDescriptorType>(binding->descriptor_type);
			info.stages = static_cast<VkShaderStageFlags>(reflectModule.shader_stage);

			bindingInfos.push_back(info);
		}
	}

	if (bindingInfos.size() > 0) {
		moduleInfo->numDescriptorInfos = bindingInfos.size();
		moduleInfo->pDescriptorInfos = new DescriptorBindingInfo[bindingInfos.size()];
		memcpy(moduleInfo->pDescriptorInfos, bindingInfos.data(), sizeof(DescriptorBindingInfo) * bindingInfos.size());
	}
}

void ShaderModuleInfo::reflectPushConstants(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo) {
	uint32_t pushConstantCount = 0;
	spvReflectEnumeratePushConstantBlocks(&reflectModule, &pushConstantCount, nullptr);

	std::vector<SpvReflectBlockVariable*> pushConstants(pushConstantCount);
	spvReflectEnumeratePushConstantBlocks(&reflectModule, &pushConstantCount, pushConstants.data());
	std::vector<PushConstantInfo> pcInfos = {};
	for (auto* pc : pushConstants) {
		PushConstantInfo pcInfo = {};
		pcInfo.offset = pc->offset;
		pcInfo.size = pc->size;
		pcInfo.stages = static_cast<VkShaderStageFlags>(reflectModule.shader_stage);

		pcInfos.push_back(pcInfo);
	}

	if (pcInfos.size() > 0) {
		moduleInfo->numPushConstantInfos = pcInfos.size();
		moduleInfo->pPushConstantInfos = new PushConstantInfo[pcInfos.size()];
		memcpy(moduleInfo->pPushConstantInfos, pcInfos.data(), sizeof(PushConstantInfo) * pcInfos.size());
	}
}

void ShaderModuleInfo::reflectVertexInputAttributeInfo(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo) {
	uint32_t inputCount = 0;
	spvReflectEnumerateInputVariables(&reflectModule, &inputCount, nullptr);

	std::vector<SpvReflectInterfaceVariable*> inputs(inputCount);
	spvReflectEnumerateInputVariables(&reflectModule, &inputCount, inputs.data());
	std::vector<VertexAttributeInfo> attributeInfos = {};
	uint32_t currOffset = 0;
	for (auto* input : inputs) {
		if (input->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
			continue;
		}

		VertexAttributeInfo attribInfo = {};
		attribInfo.location = input->location;
		attribInfo.format = static_cast<VkFormat>(input->format);
		attribInfo.offset = currOffset;

		attributeInfos.push_back(attribInfo);
		currOffset += sizeof(float) * 3;
	}

	if (attributeInfos.size() > 0) {
		moduleInfo->numVertexInputAttributeInfos = attributeInfos.size();
		moduleInfo->pVertexInputAttributeInfos = new VertexAttributeInfo[attributeInfos.size()];
		memcpy(moduleInfo->pVertexInputAttributeInfos, attributeInfos.data(), sizeof(VertexAttributeInfo) * attributeInfos.size());
	}
}

void ShaderModuleInfo::reflectFragmentOutputInfo(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo) {
	uint32_t outputCount = 0;
	spvReflectEnumerateOutputVariables(&reflectModule, &outputCount, nullptr);

	std::vector<SpvReflectInterfaceVariable*> outputs(outputCount);
	spvReflectEnumerateOutputVariables(&reflectModule, &outputCount, outputs.data());
	std::vector<FragmentOutputInfo> outputInfos = {};
	for (auto* output : outputs) {
		if (output->decoration_flags & SPV_REFLECT_DECORATION_BUILT_IN) {
			continue;
		}

		FragmentOutputInfo outputInfo = {};
		outputInfo.location = output->location;
		outputInfo.format = static_cast<VkFormat>(output->format);
		outputInfos.push_back(outputInfo);
	}

	uint32_t infoCount = outputInfos.size();
	if (infoCount > 0) {
		moduleInfo->numFragmentOutputInfos = infoCount;
		moduleInfo->pFragmentOutputInfos = new FragmentOutputInfo[infoCount];
		memcpy(moduleInfo->pFragmentOutputInfos, outputInfos.data(), sizeof(FragmentOutputInfo) * infoCount);
	}
}

void ShaderModuleInfo::reflectSpecializationConstantInfo(SpvReflectShaderModule& reflectModule, ShaderModuleInfo* moduleInfo) {
	uint32_t specCount = 0;
	spvReflectEnumerateSpecializationConstants(&reflectModule, &specCount, nullptr);

	std::vector<SpvReflectSpecializationConstant*> specs(specCount);
	spvReflectEnumerateSpecializationConstants(&reflectModule, &specCount, specs.data());
	std::vector<SpecializationConstantInfo> specInfos = {};
	for (auto* spec : specs) {
		SpecializationConstantInfo specInfo = {};
		specInfo.id = spec->constant_id;
		specInfo.size = spec->default_value_size;
		specInfos.push_back(specInfo);
	}

	uint32_t infoCount = specInfos.size();
	if (infoCount > 0) {
		moduleInfo->numSpecializationConstantInfos = infoCount;
		moduleInfo->pSpecializationConstantInfos = new SpecializationConstantInfo[infoCount];
		memcpy(moduleInfo->pSpecializationConstantInfos, specInfos.data(), sizeof(SpecializationConstantInfo) * infoCount);
	}
}