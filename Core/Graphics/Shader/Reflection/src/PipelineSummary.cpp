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
#include <unordered_map>
#include <iostream>

static uint32_t iterateDescriptorBindings(ShaderModuleInfo* stage, std::unordered_map<uint64_t, DescriptorBindingInfo>* bindingMap) {
	static auto mapKey = [](uint32_t setIndex, uint32_t bindingIndex) { return (uint64_t(setIndex) << 32) | bindingIndex; };
	uint32_t numUnique = 0;
	for (uint32_t i = 0; i < stage->numDescriptorInfos; i++) {
		DescriptorBindingInfo info = stage->pDescriptorInfos[i];
		uint64_t key = mapKey(info.setIndex, info.bindingIndex);

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

	summary->numDescriptorBindingInfos = numDescs;
	if (numDescs > 0) {
		summary->pDescriptorBindingInfos = new DescriptorBindingInfo[numDescs];
		uint32_t currentIndex = 0;
		for (auto& it : bindingMap) {
			summary->pDescriptorBindingInfos[currentIndex++] = it.second;
		}
	}
	summary->numPushConstantInfos = numPCs;
	if (numPCs > 0) {
		summary->pPushConstantInfos = new PushConstantInfo[numPCs];
		memcpy(summary->pPushConstantInfos, ranges.data(), sizeof(PushConstantInfo) * ranges.size());
	}
	return summary;
}