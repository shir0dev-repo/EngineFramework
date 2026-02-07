#pragma once

typedef unsigned int uint32_t;
typedef enum VkDescriptorType;

struct VkDescriptorSetLayout_T;
typedef VkDescriptorSetLayout_T* VkDescriptorSetLayout;

struct VkDescriptorSet_T;
typedef VkDescriptorSet_T* VkDescriptorSet;

struct DescriptorHandle {
	VkDescriptorSetLayout vkSetLayout = nullptr;
	VkDescriptorSet* vkDescriptorSets = nullptr;
	
	VkDescriptorType type;
	
	uint32_t setIndex;
	uint32_t binding;
};