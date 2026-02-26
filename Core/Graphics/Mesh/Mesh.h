#pragma once

#include "shml/vec4f.hpp"

typedef unsigned int uint32_t;

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

struct Vertex {
	alignas(16) shml::vec4f position;
	alignas(16) shml::vec4f texcoord;
	alignas(16) shml::vec4f normal;
	alignas(16) shml::vec4f color;

	static void getBindingDescription(VkVertexInputBindingDescription*& description);
	static void getAttributeDescriptions(uint32_t* count, VkVertexInputAttributeDescription*& descriptions);
private:
	static VkVertexInputBindingDescription bindingDesc;
	static VkVertexInputAttributeDescription attribDescs[4];
};

struct Mesh {
	Vertex* vertexData;
	uint32_t vertexCount;

	uint32_t* indexData;
	uint32_t indexCount;
};