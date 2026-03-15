#include "Core/UI/TextMesh.h"

#include "Core/Vulkan/VulkanInstance.h"
#include "Core/Graphics/Renderer/Renderer.h"
#include "Core/Graphics/Pipeline/GraphicsPipeline.h"
#include "Core/Graphics/Mesh/Mesh.h"
#include "Core/Graphics/Buffer/MeshBuffer.h"
#include "Core/Graphics/Buffer/GPUBuffer.h"
#include "Core/Graphics/Material/Material.h"
#include "Core/UI/Rect.h"
#include "Core/UI/FontAsset.h"

#include "shml/vec3f.hpp"

#include <map>
#include <set>
#include <vector>

#define DEFAULT_STARTING_CHARACTER 32

TextMesh* TextMesh::generate(const VulkanInstance* const instance, Renderer* const renderer, Material* const materialRef, 
	uint32_t windowWidth, uint32_t windowHeight, FontAsset* const font, const std::string text, Rect rect, float fontSize) {

	float pixelScale = 2.0f / windowHeight;
	shml::vec3f startPosition {
		rect.x * pixelScale * fontSize,
		rect.y * font->fontSize * pixelScale * fontSize,
		0.0f
	};
	
	startPosition.x -= (windowWidth / windowHeight);
	startPosition.y -= 1.0 - (font->fontSize * pixelScale * fontSize);

	shml::vec3f localPosition = startPosition;
	char currentChar;

	std::vector<Vertex> vertices{};
	std::vector<uint32_t> indices{};
	uint32_t currentVertexIndex = 0;

	for (uint32_t i = 0; i < text.size(); i++) {
		currentChar = text[i];
		if (currentChar == '\n') {
			localPosition.y += font->fontSize * pixelScale * fontSize;
			localPosition.x = startPosition.x;
			continue;
		}
		else if (currentChar < DEFAULT_STARTING_CHARACTER || currentChar > DEFAULT_STARTING_CHARACTER + font->numGlyphs) {
			continue;
		}

		stbtt_packedchar glyph = font->packedChars[currentChar - DEFAULT_STARTING_CHARACTER];
		stbtt_aligned_quad quad = font->alignedQuads[currentChar - DEFAULT_STARTING_CHARACTER];

		shml::vec3f glyphSize {
			(glyph.x1 - glyph.x0) * pixelScale * fontSize,
			(glyph.y1 - glyph.y0) * pixelScale * fontSize,
			0.0
		};

		// starting position is top left corner

		Vertex bottomLeft{};
		bottomLeft.position.x = localPosition.x + (glyph.xoff * pixelScale * fontSize);
		bottomLeft.position.y = localPosition.y + (glyph.y1 - glyph.y0 + glyph.yoff) * pixelScale * fontSize;
		bottomLeft.normal = { 0.0f, 0.0f, 1.0f, 0.0f };
		bottomLeft.texcoord.x = quad.s0;
		bottomLeft.texcoord.y = quad.t1;

		Vertex topRight{};
		topRight.position.x = bottomLeft.position.x + glyphSize.x;
		topRight.position.y = bottomLeft.position.y + glyphSize.y;
		topRight.normal = { 0.0f, 0.0f, 1.0f, 0.0f };
		topRight.texcoord.x = quad.s1;
		topRight.texcoord.y = quad.t0;

		Vertex topLeft{};
		topLeft.position = bottomLeft.position;
		topLeft.position.y += glyphSize.y;
		topLeft.normal = { 0.0f, 0.0f, 1.0f, 0.0f };
		topLeft.texcoord.x = quad.s0;
		topLeft.texcoord.y = quad.t0;

		Vertex bottomRight{};
		bottomRight.position = bottomLeft.position;
		bottomRight.position.x += glyphSize.x;
		bottomRight.normal = { 0.0f, 0.0f, 1.0f, 0.0f };
		bottomRight.texcoord.x = quad.s1;
		bottomRight.texcoord.y = quad.t1;

		bottomLeft.position.y *= -1;
		topRight.position.y *= -1;
		topLeft.position.y *= -1;
		bottomRight.position.y *= -1;

		vertices.push_back(topRight);
		vertices.push_back(topLeft);
		vertices.push_back(bottomLeft);
		vertices.push_back(bottomRight);
		
		indices.push_back(currentVertexIndex + 0);
		indices.push_back(currentVertexIndex + 1);
		indices.push_back(currentVertexIndex + 2);
		indices.push_back(currentVertexIndex + 0);
		indices.push_back(currentVertexIndex + 2);
		indices.push_back(currentVertexIndex + 3);

		currentVertexIndex += 4;
		localPosition.x += glyph.xadvance * pixelScale * fontSize;
	}

	// Create temp. mesh to store data
	Mesh* m = new Mesh();
	m->vertexData = vertices.data();
	m->vertexCount = vertices.size();
	m->indexData = indices.data();
	m->indexCount = indices.size();

	TextMesh* textMesh = new TextMesh();
	textMesh->text = Text(text);

	textMesh->meshBuffer = new MeshBuffer();
	textMesh->meshBuffer->setup(instance, renderer, m);
	
	textMesh->material = materialRef;
	textMesh->transformBuffer = nullptr;

	return textMesh;
}

void TextMesh::setup(const VulkanInstance* const instance, Renderer* renderer, std::string text, Mesh* const meshRef, Material* const materialRef) {
	IRenderable::setup(instance, renderer, nullptr, meshRef, materialRef);
	this->text = text;
}

void TextMesh::draw() {
	if (!material) {
		return;
	}
	material->pipeline->addRenderCommand(this);
}

void TextMesh::teardown(const VulkanInstance* const instance) {
	if (meshBuffer != nullptr) {
		meshBuffer->teardown(instance);
		meshBuffer = nullptr;
	}
	if (transformBuffer != nullptr) {
		transformBuffer->dispose(instance->logicalDevice);
		transformBuffer = nullptr;
	}
}