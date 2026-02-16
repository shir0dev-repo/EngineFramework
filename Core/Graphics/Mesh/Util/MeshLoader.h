#pragma once

struct Mesh;

struct MeshLoader {
	static bool loadOBJ(const char* filePath, Mesh** outMesh);
};