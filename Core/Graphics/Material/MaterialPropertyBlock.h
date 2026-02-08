#pragma once

struct Material;

struct MaterialPropertyBlock {
	void* bufferedData = nullptr;

	void setupPropertyBlock(Material* material);
};