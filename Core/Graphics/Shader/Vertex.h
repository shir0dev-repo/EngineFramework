#pragma once

#include "shml/vec3f.hpp"

typedef unsigned int uint32_t;

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

struct Vertex {
	shml::vec3f position;
	shml::vec3f texcoord;
	shml::vec3f normal;
	shml::vec3f color;

	static void getBindingDescription(VkVertexInputBindingDescription*& description);
	static void getAttributeDescriptions(uint32_t* count, VkVertexInputAttributeDescription*& descriptions);
private:
	static VkVertexInputBindingDescription bindingDesc;
	static VkVertexInputAttributeDescription attribDescs[4];
};