#pragma once

typedef unsigned int uint32_t;
struct Vertex;

struct Mesh {
	Vertex* vertexData;
	uint32_t vertexCount;

	uint32_t* indexData;
	uint32_t indexCount;
};