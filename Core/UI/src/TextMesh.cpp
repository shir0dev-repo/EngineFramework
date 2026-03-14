#include "Core/UI/TextMesh.h"

#include "Core/Vulkan/VulkanInstance.h"
#include "Core/Graphics/Renderer/Renderer.h"
#include "Core/Graphics/Pipeline/GraphicsPipeline.h"
#include "Core/Graphics/Mesh/Mesh.h"
#include "Core/Graphics/Buffer/MeshBuffer.h"
#include "Core/Graphics/Material/Material.h"
#include "Core/UI/Rect.h"
#include "Core/UI/FontAsset.h"

#include "shml/vec3f.hpp"

void TextMesh::setup(const VulkanInstance* const instance, Renderer* renderer, std::string text, Mesh* const meshRef, Material* const materialRef) {
	IRenderable::setup(instance, renderer, nullptr, meshRef, materialRef);
	this->text = text;
}

TextMesh* TextMesh::generate(const VulkanInstance* const instance, uint32_t windowWidth, uint32_t windowHeight,
	FontAsset* const font, const std::string text, Rect rect, float fontSize) {

	float pixelScale = 2.0f / windowHeight;
	shml::vec3f position;
	
	float pivotX = rect.x;
	float pivotY = rect.y;
	float endX = pivotX + rect.width;
	float endY = pivotY + rect.height;

	float charWidth = fabsf(pivotX - rect.width) / font->atlasSize;
	return nullptr;
}

void TextMesh::draw() {
	if (!material) {
		return;
	}
	material->pipeline->addRenderCommand(this);
}