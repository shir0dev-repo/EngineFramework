#pragma once

#include "Core/UI/Text.h"
#include "Core/Entity/Component/IRenderable.h"
#include "Core/UI/Rect.h"
struct VulkanInstance;
struct Renderer;
struct Mesh;
struct MeshBuffer;
struct Material;
struct FontAsset;

struct TextMesh : public IRenderable {
	Text text;
	MeshBuffer* mesh;
	Material* material;

	static TextMesh* generate(const VulkanInstance* const instance, uint32_t windowWidth, uint32_t windowHeight,
		FontAsset* const font, const std::string text, Rect rect, float fontSize);

	void setup(const VulkanInstance* const instance, Renderer* renderer, std::string text, Mesh* const meshRef, Material* const materialRef);
	virtual void draw() override;
};