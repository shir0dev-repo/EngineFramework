#include "../Vertex.h"

#include <vulkan/vulkan.h>

static const uint32_t ATTRIB_COUNT = 4;

VkVertexInputAttributeDescription Vertex::attribDescs[ATTRIB_COUNT]{};
VkVertexInputBindingDescription Vertex::bindingDesc{};

void Vertex::getBindingDescription(VkVertexInputBindingDescription*& description) {
	static bool hasBeenRetrieved = false;
	if (!hasBeenRetrieved) {
		bindingDesc.binding = 0;
		bindingDesc.stride = sizeof(Vertex);
		bindingDesc.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		hasBeenRetrieved = true;
	}
	
	description = &bindingDesc;
}

void Vertex::getAttributeDescriptions(uint32_t* count, VkVertexInputAttributeDescription*& descs) {
	static bool hasBeenRetrieved = false;
	if (!hasBeenRetrieved) {
		// position attribute
		attribDescs[0].binding = 0;
		attribDescs[0].location = 0;
		attribDescs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDescs[0].offset = 0;
		// texcoord attribute
		attribDescs[1].binding = 0;
		attribDescs[1].location = 1;
		attribDescs[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDescs[1].offset = sizeof(shml::vec3f);
		// normal attribute
		attribDescs[2].binding = 0;
		attribDescs[2].location = 2;
		attribDescs[2].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDescs[2].offset = sizeof(shml::vec3f) * 2;
		// color attribute
		attribDescs[3].binding = 0;
		attribDescs[3].location = 3;
		attribDescs[3].format = VK_FORMAT_R32G32B32_SFLOAT;
		attribDescs[3].offset = sizeof(shml::vec3f) * 3;

		hasBeenRetrieved = true;
	}

	*count = ATTRIB_COUNT;
	descs = &attribDescs[0];
}