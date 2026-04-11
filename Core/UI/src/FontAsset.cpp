#include "Core/UI/FontAsset.h"
#include "Core/Graphics/Texture/GPUTexture.h"
#include "Core/Vulkan/VulkanContext.h"
#include "STBTT/stb_truetype.h"
#include <fstream>
#include <iostream>
#include <map>
#include <shml/vec4f.hpp>

#define DEFAULT_START_CHARACTER 'a'

static std::map<std::string, FontAsset*> fontMap;

FontAsset* const FontAsset::create(const VulkanContext* const instance, const char* filePath, const float pointSize) {
	uint8_t* fontBuffer = nullptr;
	stbtt_fontinfo fontInfo;

	try {
		std::ifstream inputStream(filePath, std::ios::in | std::ios::binary);

		inputStream.seekg(0, std::ios::end);
		std::streampos fileSize = inputStream.tellg();
		uint32_t sz = fileSize;
		inputStream.seekg(0, std::ios::beg);

		fontBuffer = new uint8_t[fileSize];
		inputStream.read((char*)fontBuffer, fileSize);
		inputStream.close();

		if (!stbtt_InitFont(&fontInfo, fontBuffer, 0)) {
			std::cout << "Failed to create font!\n";
		}
	}
	catch (std::ifstream::failure& e) {
		std::cout << "Failed to read font file, " << e.what() << std::endl;
		return nullptr;
	}
	uint32_t closestPow2 = (uint32_t)(pointSize * sqrtf(fontInfo.numGlyphs)) - 1;
	closestPow2 |= closestPow2 >> 1;
	closestPow2 |= closestPow2 >> 2;
	closestPow2 |= closestPow2 >> 4;
	closestPow2 |= closestPow2 >> 8;
	closestPow2 |= closestPow2 >> 16;
	closestPow2++;

	FontAsset* font = new FontAsset();
	
	font->atlasSize = closestPow2;
	font->numGlyphs = fontInfo.numGlyphs;
	font->fontSize = pointSize;
	font->packedChars = new stbtt_packedchar[font->numGlyphs];
	font->alignedQuads = new stbtt_aligned_quad[font->numGlyphs];
	font->fontAtlasTextureData = new uint8_t[font->atlasSize * font->atlasSize];
	stbtt_pack_context packContext;
	stbtt_PackBegin(&packContext, font->fontAtlasTextureData, font->atlasSize, font->atlasSize, font->atlasSize, 1, nullptr);
	
	stbtt_PackFontRange(&packContext, fontBuffer, 0, font->fontSize, 32, font->numGlyphs, font->packedChars);
	stbtt_PackEnd(&packContext);

	for (uint32_t i = 0; i < font->numGlyphs; i++) {
		float posX, posY;
		stbtt_GetPackedQuad(font->packedChars, font->atlasSize, font->atlasSize, i, &posX, &posY, &font->alignedQuads[i], 0);
	}
	
	uint32_t* pixels = new uint32_t[font->atlasSize * font->atlasSize];
	for (uint32_t i = 0; i < font->atlasSize * font->atlasSize; i++) {
		uint32_t alpha = 0;
		alpha |= font->fontAtlasTextureData[i] << 24;
		pixels[i] = alpha;
	}
	uint32_t sizeInBytes = sizeof(uint32_t) * font->atlasSize * font->atlasSize;
	font->texture = GPUTexture::createTextureLoadImmediate(instance, font->atlasSize, font->atlasSize, 4, sizeInBytes, pixels, "default-font");
	delete[] pixels;
	fontMap.emplace("default", font);
	return font;
}

FontAsset* const FontAsset::find(const char* name) {
	auto it = fontMap.find(name);
	if (it != fontMap.end()) {
		return it->second;
	}
	else {
		return nullptr;
	}
}