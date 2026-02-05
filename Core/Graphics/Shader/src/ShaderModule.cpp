#include "../ShaderModule.h"
#include "../Reflection/DescriptorBindingInfo.h"
#include "../Reflection/VertexAttributeInfo.h"
#include "../Reflection/PushConstantInfo.h"
#include "../Reflection/FragmentOutputInfo.h"
#include "../Reflection/SpecializationConstantInfo.h"

#include "../ShaderModuleInfo.h"
#include "SPIRV-Reflect/spirv_reflect.h"

#include <vulkan/vulkan.h>
#include <cassert>
#include <cstring>
#include <fstream>
#include <map>

static std::map<const char*, const char*> nameToFilePathMap;
static std::map<const char*, ShaderModule*> filePathShaders;
static std::map<const char*, ShaderModuleInfo> shaderInfos;

void ShaderModule::readShader(const char* filePath, char*& outFileContents, uint32_t* fileSize) {
	std::ifstream file(filePath, std::ios::ate | std::ios::binary);
	
	if (!file.is_open()) {
		char buf[100];
		sprintf_s(buf, 100, "Failed to open file: %s", filePath);
		throw std::runtime_error(buf);
	}

	*fileSize = (size_t) file.tellg();
	if (outFileContents != nullptr) {
		delete[] outFileContents;
	}

	outFileContents = new char[*fileSize];
	file.seekg(0);
	file.read(outFileContents, *fileSize);
	file.close();
}

const char* ShaderModule::getName() const {
	return name;
}
VkShaderModule_T* const ShaderModule::getModule() const {
	return module;
}
ShaderModuleInfo* const ShaderModule::getInfo() const {
	return moduleInfo;
}

bool ShaderModule::find(const char* name, ShaderModule** pShader) {
	auto it = nameToFilePathMap.find(name);
	if (it != nameToFilePathMap.end()) {
		auto itShader = filePathShaders.find(it->second);
		if (itShader != filePathShaders.end()) {
			*pShader = itShader->second;
		}
	}
	return *pShader != nullptr;
}

bool ShaderModule::getInfo(const ShaderModule& shader, ShaderModuleInfo** pInfo) {
	auto it = nameToFilePathMap.find(shader.name);
	if (it != nameToFilePathMap.end()) {
		auto itInfo = shaderInfos.find(it->second);
		if (itInfo != shaderInfos.end()) {
			*pInfo = &itInfo->second;
		}
	}
	return pInfo != nullptr;
}

ShaderModule* const ShaderModule::createNew(VkDevice_T* logicalDevice, const char* filePath, const char* name) {
	ShaderModule* shader = nullptr;

	uint32_t len = strlen(name);
	char* nameBuffer = nullptr;
	
	if (find(filePath, &shader)) {
		return shader;
	}
	else if (strcmp(name, "") == 0) {
		nameBuffer = new char[32];
		sprintf_s(nameBuffer, 32, "Unnamed Shader %l", filePathShaders.size() + 1);
	}
	else {
		nameBuffer = new char[len + 1];
		strcpy_s(nameBuffer, uint64_t(len + 1), name);
		nameBuffer[len] = 0;
	}

	char* shaderBytes = nullptr;
	uint32_t shaderByteLen = 0;
	readShader(filePath, shaderBytes, &shaderByteLen);

	shader = new ShaderModule();
	shader->name = nameBuffer;
	ShaderModuleInfo moduleInfo = ShaderModuleInfo::createModuleInfo(shaderBytes, shaderByteLen);
	createModule(logicalDevice, shaderBytes, shaderByteLen, &shader->module);

	nameToFilePathMap.emplace(nameBuffer, filePath);
	filePathShaders.emplace(filePath, shader);
	shaderInfos.emplace(filePath, moduleInfo);
	
	return shader;
}

void ShaderModule::createModule(VkDevice_T* logicalDevice, const char* byteCode, const uint32_t& byteLen, VkShaderModule_T** outModule) {
	VkShaderModuleCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = byteLen;
	createInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode);

	if (vkCreateShaderModule(logicalDevice, &createInfo, nullptr, outModule) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create shader module!");
	}
}

void ShaderModule::teardown(VkDevice_T* logicalDevice) {
	for (auto& [name, module] : filePathShaders) {
		vkDestroyShaderModule(logicalDevice, module->getModule(), nullptr);
	}
}