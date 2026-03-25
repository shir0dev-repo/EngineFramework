#pragma once
#include "STBTT/stb_truetype.h"
#define STB_TRUETYPE_IMPLEMENTATION

struct FontAsset {
	unsigned int numGlyphs = 0;
	unsigned int atlasSize = 0;
	float fontSize = 0.0f;

	unsigned char* fontAtlasTextureData = nullptr;
	stbtt_aligned_quad* alignedQuads = nullptr;
	stbtt_packedchar* packedChars = nullptr;
	struct GPUTexture* texture = nullptr;

	static FontAsset* const create(const struct VulkanInstance* const instance, const char* path, const float pointSize = 12.0f);
	static FontAsset* const find(const char* name);
};