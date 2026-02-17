#pragma once

typedef unsigned int uint32_t;

struct VkDescriptorSet_T;
struct VkCommandBuffer_T;

struct VulkanInstance;
struct PipelineSummary;
struct GraphicsPipeline;
struct GPUTexture;
struct GPUBuffer;
struct MaterialBinding;

struct Material {
	
	struct TextureHandleEntry {
		GPUTexture* defaultHandle = nullptr;
		union {
			bool isCustomHandle;
			GPUTexture* customHandle = nullptr;
		};
		uint32_t binding = 0;
	}* pTextures = nullptr;
	
	struct BufferHandleEntry {
		uint32_t binding = 0;
		GPUBuffer* handle = nullptr;
	};

	BufferHandleEntry* pBuffers = nullptr;
	BufferHandleEntry* pInstanceBuffers = nullptr;

	GraphicsPipeline* pipeline = nullptr;

	uint32_t numTextureHandles = 0;
	uint32_t numBufferHandles = 0;
	uint32_t numInstanceBufferHandles = 0;

	VkDescriptorSet_T** vkDescriptorSets = nullptr;
	VkDescriptorSet_T** vkInstanceDescriptorSets = nullptr;

	static Material* const create(const VulkanInstance* const instance, GraphicsPipeline* const pipeline, const char* name);
	static bool find(const char* name, Material** outMaterial);
	static void cleanup(const VulkanInstance* const instance);

	void bind(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) const;
	void bindInstance(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) const;

	void unbind(VkCommandBuffer_T* commandBuffer, uint32_t currentFrame) const;

	void setTexture(const VulkanInstance* const instance, GPUTexture* texture, uint32_t binding);
	void setFloat(const VulkanInstance* const instance, uint32_t binding, float value) const;
	void setBuffer(const VulkanInstance* const instance, uint32_t set, uint32_t binding, const void* data, uint32_t size) const;
private:
	void createTextureHandles(const VulkanInstance* const instance, GraphicsPipeline* const pipeline, const PipelineSummary* const summary,
		MaterialBinding* bindings, uint32_t bindingCount);
	void createBufferHandles(const VulkanInstance* const instance, GraphicsPipeline* const pipeline, const PipelineSummary* const summary,
		MaterialBinding* bindings, uint32_t bindingCount);
	void createInstanceBufferHandles(const VulkanInstance* const instance, GraphicsPipeline* const pipeline, const PipelineSummary* const summary,
		MaterialBinding* bindings, uint32_t bindingCount);
};