#include "../PipelineSummary.h"
#include "../DescriptorBindingInfo.h"
#include "../PushConstantInfo.h"
#include "../VertexAttributeInfo.h"
#include "../FragmentOutputInfo.h"
#include "../../ShaderModule.h"
#include "../../ShaderModuleInfo.h"

#include "SPIRV-Reflect/spirv_reflect.h"

#include <vulkan/vulkan.h>
#include <vector>
#include <map>
#include <unordered_map>
#include <iostream>

static uint64_t makeDescriptorMapKey(uint32_t setIndex, uint32_t bindingIndex) {
	return (uint64_t(setIndex) << 32) | bindingIndex;
}

static uint32_t iterateDescriptorBindings(ShaderModuleInfo* stage, std::unordered_map<uint64_t, DescriptorBindingInfo>* bindingMap) {
	
	uint32_t numUnique = 0;
	for (uint32_t i = 0; i < stage->globalDescriptors.count; i++) {
		DescriptorBindingInfo info = stage->globalDescriptors.pDescriptorBindingInfos[i];
		uint64_t key = makeDescriptorMapKey(info.setIndex, info.bindingIndex);

		auto it = bindingMap->find(key);
		if (it == bindingMap->end()) {
			numUnique++;
			(*bindingMap)[key] = info;
		}
		else if (it->second.type != info.type || it->second.count != info.count) {
			throw std::runtime_error("Descriptor binding mismatch between stages!");
		}
		else {
			it->second.stages |= info.stages;
		}
	}
	for (uint32_t i = 0; i < stage->pipelineDescriptors.count; i++) {
		DescriptorBindingInfo info = stage->pipelineDescriptors.pDescriptorBindingInfos[i];
		uint64_t key = makeDescriptorMapKey(info.setIndex, info.bindingIndex);

		auto it = bindingMap->find(key);
		if (it == bindingMap->end()) {
			numUnique++;
			(*bindingMap)[key] = info;
		}
		else if (it->second.type != info.type || it->second.count != info.count) {
			throw std::runtime_error("Descriptor binding mismatch between stages!");
		}
		else {
			it->second.stages |= info.stages;
		}
	}
	for (uint32_t i = 0; i < stage->materialDescriptors.count; i++) {
		DescriptorBindingInfo info = stage->materialDescriptors.pDescriptorBindingInfos[i];
		uint64_t key = makeDescriptorMapKey(info.setIndex, info.bindingIndex);

		auto it = bindingMap->find(key);
		if (it == bindingMap->end()) {
			numUnique++;
			(*bindingMap)[key] = info;
		}
		else if (it->second.type != info.type || it->second.count != info.count) {
			throw std::runtime_error("Descriptor binding mismatch between stages!");
		}
		else {
			it->second.stages |= info.stages;
		}
	}
	for (uint32_t i = 0; i < stage->instanceDescriptors.count; i++) {
		DescriptorBindingInfo info = stage->instanceDescriptors.pDescriptorBindingInfos[i];
		uint64_t key = makeDescriptorMapKey(info.setIndex, info.bindingIndex);

		auto it = bindingMap->find(key);
		if (it == bindingMap->end()) {
			numUnique++;
			(*bindingMap)[key] = info;
		}
		else if (it->second.type != info.type || it->second.count != info.count) {
			throw std::runtime_error("Descriptor binding mismatch between stages!");
		}
		else {
			it->second.stages |= info.stages;
		}
	}
	return numUnique;
}

static uint32_t iteratePushConstants(ShaderModuleInfo* stage, std::vector<PushConstantInfo>* ranges) {
	uint32_t numUnique = 0;
	for (uint32_t i = 0; i < stage->numPushConstantInfos; i++) {
		PushConstantInfo pc = stage->pPushConstantInfos[i];
		bool merged = false;
		for (auto& r : *ranges) {
			bool overlap = pc.offset < r.offset + r.size && r.offset < pc.offset + pc.size;
			if (overlap) {
				uint32_t start = std::min(r.offset, pc.offset);
				uint32_t end = std::max(r.offset + r.size, pc.offset + pc.size);

				r.offset = start;
				r.size = end - start;
				r.stages |= pc.stages;
				merged = true;
				break;
			}
		}

		if (merged == false) {
			ranges->push_back({ pc.offset, pc.size, pc.stages });
			numUnique++;
		}
	}

	return numUnique;
}

PipelineSummary* PipelineSummary::createSummary(ShaderModule** const stages, uint32_t numStages) {
	PipelineSummary* summary = new PipelineSummary();
	std::unordered_map<uint64_t, DescriptorBindingInfo> bindingMap;
	std::vector<PushConstantInfo> ranges;
	uint32_t numDescs = 0, numPCs = 0;

	for (uint32_t i = 0; i < numStages; i++) {
		ShaderModuleInfo* stage = nullptr;
		if (!ShaderModule::getInfo(*stages[i], &stage)) {
			delete summary;
			throw std::runtime_error("Failed to get info for shader stage!");
		}
		
		numDescs += iterateDescriptorBindings(stage, &bindingMap);
		numPCs += iteratePushConstants(stage, &ranges);
		
		if (stage->stage & VK_SHADER_STAGE_VERTEX_BIT) {
			uint32_t vCount = stage->numVertexInputAttributeInfos;
			if (vCount > 0) {
				summary->numVertexInputAttributeInfos = vCount;
				summary->pVertexInputAttributeInfos = new VertexAttributeInfo[vCount];
				memcpy(summary->pVertexInputAttributeInfos, stage->pVertexInputAttributeInfos, sizeof(VertexAttributeInfo) * vCount);
			}
		}
		else if (stage->stage & VK_SHADER_STAGE_FRAGMENT_BIT) {
			uint32_t oCount = stage->numFragmentOutputInfos;
			if (oCount > 0) {
				summary->numFragmentOutputInfos = oCount;
				summary->pFragmentOutputInfos = new FragmentOutputInfo[oCount];
				memcpy(summary->pFragmentOutputInfos, stage->pFragmentOutputInfos, sizeof(FragmentOutputInfo) * oCount);
			}
		}
	}

	std::map<uint32_t, std::vector<DescriptorBindingInfo>> setBindings;
	setBindings[0] = {};
	setBindings[1] = {};
	setBindings[2] = {};
	setBindings[3] = {};

	if (numDescs > 0) {
		for (auto& [setBindingMask, binding] : bindingMap) {
			uint32_t set = static_cast<uint32_t>((setBindingMask & 0xFFFFFFFF00000000) >> 32);
			uint32_t bindingIndex = static_cast<uint32_t>((setBindingMask & 0xFFFFFFFF));
			switch (set) {
				case 0:
					summary->globalDescriptors.count++;
					setBindings[0].push_back(binding);
					break;
				case 1:
					summary->pipelineDescriptors.count++;
					setBindings[1].push_back(binding);
					break;
				case 2:
					summary->materialDescriptors.count++;
					setBindings[2].push_back(binding);
					break;
				case 3:
					summary->instanceDescriptors.count++;
					setBindings[3].push_back(binding);
					break;
				default:
					throw std::runtime_error("Unsupported set found!");
			}
		}

		if (setBindings[0].size() > 0) {
			summary->globalDescriptors.count = setBindings[0].size();
			summary->globalDescriptors.pDescriptorBindingInfos = new DescriptorBindingInfo[setBindings[0].size()];
			memcpy(summary->globalDescriptors.pDescriptorBindingInfos, setBindings[0].data(), sizeof(DescriptorBindingInfo) * setBindings[0].size());
		}
		if (setBindings[1].size() > 0) {
			summary->pipelineDescriptors.count = setBindings[1].size();
			summary->pipelineDescriptors.pDescriptorBindingInfos = new DescriptorBindingInfo[setBindings[1].size()];
			memcpy(summary->pipelineDescriptors.pDescriptorBindingInfos, setBindings[1].data(), sizeof(DescriptorBindingInfo) * setBindings[1].size());
		}
		if (setBindings[2].size() > 0) {
			summary->materialDescriptors.count = setBindings[2].size();
			summary->materialDescriptors.pDescriptorBindingInfos = new DescriptorBindingInfo[setBindings[2].size()];
			memcpy(summary->materialDescriptors.pDescriptorBindingInfos, setBindings[2].data(), sizeof(DescriptorBindingInfo) * setBindings[2].size());
		}
		if (setBindings[3].size() > 0) {
			summary->instanceDescriptors.count = setBindings[3].size();
			summary->instanceDescriptors.pDescriptorBindingInfos = new DescriptorBindingInfo[setBindings[3].size()];
			memcpy(summary->instanceDescriptors.pDescriptorBindingInfos, setBindings[3].data(), sizeof(DescriptorBindingInfo) * setBindings[3].size());
		}
	}
	summary->numPushConstantInfos = numPCs;
	if (numPCs > 0) {
		summary->pPushConstantInfos = new PushConstantInfo[numPCs];
		memcpy(summary->pPushConstantInfos, ranges.data(), sizeof(PushConstantInfo) * ranges.size());
	}
	return summary;
}