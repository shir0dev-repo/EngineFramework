#pragma once

#include "Core/UI/Text.h"
#include "Core/Entity/Component/IRenderable.h"
#include "Core/Scene/SceneNode.h"

#include "Core/UI/Rect.h"
struct VulkanContext;
struct Renderer;
struct Mesh;
struct MeshBuffer;
struct Material;
struct FontAsset;

struct TextMesh : public IRenderable, public SceneNode {
	Text text;

	static TextMesh* generate(const VulkanContext* const instance, Renderer* const renderer, Material* const materialRef,
		uint32_t windowWidth, uint32_t windowHeight, FontAsset* const font, const std::string text, Rect rect, float fontSize);

	void setup(const VulkanContext* const instance, Renderer* renderer, std::string text, Mesh* const meshRef, Material* const materialRef);
	void teardown(const VulkanContext* const instance);
	virtual void update(class World* world, const float dt) override;
	virtual void draw() override;
};