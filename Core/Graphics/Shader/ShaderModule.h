#pragma once

typedef unsigned int uint32_t;

struct VkDevice_T;
struct VkShaderModule_T;

struct SpvReflectShaderModule;

struct ShaderModuleInfo;

struct ShaderModule {
	static ShaderModule* const createNew(VkDevice_T* logicalDevice, const char* filePath, const char* name = "");
	static bool find(const char* name, ShaderModule** pShader);
	static bool getInfo(const ShaderModule& shader, ShaderModuleInfo** pInfo);
	static void teardown(VkDevice_T* logicalDevice);

	const char* getName() const;
	VkShaderModule_T* const getModule() const;
	ShaderModuleInfo* const getInfo() const;
private:
	static void readShader(const char* filePath, char*& outFileContents, uint32_t* fileSize);
	static void createModule(VkDevice_T* logicalDevice, const char* byteCode, const uint32_t& byteLen, VkShaderModule_T** outModule);
	
	char* name = nullptr;
	VkShaderModule_T* module = nullptr;
	ShaderModuleInfo* moduleInfo = nullptr;
};