#include "../MeshLoader.h"
#include "../../Mesh.h"
#include <shml/vec3f.hpp>
#include "Core/Graphics/Shader/Vertex.h"

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <thread>
#include <unordered_map>

using StrVec = std::vector<std::string>;
template <typename T>
using Vec = std::vector<T>;
using vec3f = shml::vec3f;

struct VertexData {
	uint32_t p;
	uint32_t t;
	uint32_t n;
	inline uint32_t& operator[](const uint8_t& i) {
		if (i == 0) return p;
		if (i == 1) return t;
		if (i == 2) return n;
		else throw;
	}

	bool operator==(const VertexData& other) const {
		return p == other.p && t == other.t;
	}
};

static bool operator==(const Vertex& v, const Vertex& other) {
	return v.position == other.position && v.texcoord == other.texcoord;
}

struct FaceData {
	VertexData a;
	VertexData b;
	VertexData c;
	inline VertexData& operator[](const uint8_t& i) {
		if (i == 0) return a;
		if (i == 1) return b;
		if (i == 2) return c;
		else throw;
	}
};

namespace std {
	template<> struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			size_t p = vertex.position.x;
			p ^= (static_cast<size_t>(vertex.position.y) << 1) >> 1;
			p ^= (static_cast<size_t>(vertex.position.z) << 1) >> 1;
			size_t t = vertex.texcoord.x;
			t ^= (static_cast<size_t>(vertex.texcoord.y) << 1) >> 1;
			t ^= (static_cast<size_t>(vertex.texcoord.z) << 1) >> 1;
			size_t n = vertex.normal.x;
			n ^= (static_cast<size_t>(vertex.normal.y) << 1) >> 1;
			n ^= (static_cast<size_t>(vertex.normal.z) << 1) >> 1;
			return (p ^ (t << 1) >> 1) ^ n;
		}
	};
}

static bool parseOBJFile(const char* filePath, StrVec* vertices, StrVec* texCoords, StrVec* normals, StrVec* faces);
static void createMeshData(StrVec& vertexList, Vec<vec3f>& vertices, StrVec& texCoordList, Vec<vec3f>& texCoords, StrVec& normalList, Vec<vec3f>& normals,
	StrVec& faceList, Vec<FaceData>& faces);
static void parseVectorBatch(StrVec& stringBatch, Vec<vec3f>& outData, const uint8_t stride);
static void parseFaceBatch(StrVec& faceBatch, Vec<FaceData>& outData);
static void convertMeshData(Mesh* mesh, Vec<vec3f>& vertices, Vec<vec3f>& texCoords, Vec<vec3f>& normals, Vec<FaceData>& faces);

bool MeshLoader::loadOBJ(const char* filePath, Mesh** outMesh) {
	if (outMesh == nullptr) {
		return false;
	}

	StrVec vertexList, texCoordList, normalList, faceList;
	bool readOK = parseOBJFile(filePath, &vertexList, &texCoordList, &normalList, &faceList);
	if (!readOK) {
		return false;
	}
	
	Vec<shml::vec3f> vertices, texCoords, normals;
	Vec<FaceData> faces;
	
	createMeshData(vertexList, vertices, texCoordList, texCoords, normalList, normals, faceList, faces);
	*outMesh = new Mesh();

	convertMeshData(*outMesh, vertices, texCoords, normals, faces);
	return true;
}

bool parseOBJFile(const char* filePath, StrVec* vertices, StrVec* texCoords, StrVec* normals, StrVec* faces) {
	std::ifstream obj(filePath, std::ifstream::in);
	if (!obj.is_open()) {
		return false;
	}

	std::string currentLine;
	std::string type;

	size_t offset;
	size_t npos = std::string::npos;

	while (std::getline(obj, currentLine)) {
		offset = currentLine.find_first_of(' ');
		if (offset == npos) {
			continue;
		}
		
		type = currentLine.substr(0, offset);

		currentLine.erase(0, 2);
		currentLine.erase(0, currentLine.find_first_not_of(' '));

		if (type == "v") {
			vertices->push_back(currentLine + '\n');
		}
		else if (type == "vt") {
			texCoords->push_back(currentLine + '\n');
		}
		else if (type == "vn") {
			normals->push_back(currentLine + '\n');
		}

		else if (type == "f") {
			faces->push_back(currentLine + '\n');
		}
		else continue;
	}

	obj.close();
	return true;
}



void createMeshData(StrVec& vertexList, Vec<vec3f>& vertices, StrVec& texCoordList, Vec<vec3f>& texCoords, StrVec& normalList, Vec<vec3f>& normals,
	StrVec& faceList, Vec<FaceData>& faces) {

	std::thread positionWorker(&parseVectorBatch, std::ref(vertexList), std::ref(vertices), 3);
	printf("|-- Parsing vertex positions in thread %lu.\n", positionWorker.get_id());
	std::thread texCoordWorker(&parseVectorBatch, std::ref(texCoordList), std::ref(texCoords), 2);
	printf("|-- Parsing texture coordinates in thread %lu.\n", texCoordWorker.get_id());
	std::thread normalWorker(&parseVectorBatch, std::ref(normalList), std::ref(normals), 3);
	printf("|-- Parsing normals in thread %lu.\n", normalWorker.get_id());
	std::thread faceWorker(&parseFaceBatch, std::ref(faceList), std::ref(faces));

	positionWorker.join();
	texCoordWorker.join();
	normalWorker.join();
	faceWorker.join();
}

void parseVectorBatch(StrVec& stringBatch, Vec<vec3f>& outData, const uint8_t stride) {
	std::stringstream ss;
	for (std::string l : stringBatch) {
		ss << l;
	}
	ss << "eof";

	std::string line;
	while (true) {
		std::getline(ss, line);
		if (line == "eof") {
			break;
		}

		vec3f v;
		for (uint32_t i = 0; i < stride; i++) {
			size_t separator = line.find_first_of(' ');
			std::string valStr = line.substr(0, separator);
			v[i] = std::stof(valStr);

			line.erase(0, separator + 1);
		}

		outData.push_back(v);
	}
}

void parseFaceBatch(StrVec& faceBatch, Vec<FaceData>& outData) {
	std::stringstream ss;
	for (std::string f : faceBatch) {
		ss << f;
	}
	ss << "eof";

	std::string indices[3];
	size_t separator, end;
	size_t npos = std::string::npos;

	std::string line;
	while (true) {
		std::getline(ss, line);
		if (line == "eof") {
			break;
		}

		for (int i = 0; i < 3; i++) {
			separator = line.find_first_of(' ');
			indices[i] = line.substr(0, separator);
			line.erase(0, separator + 1);
		}

		FaceData face{};
		for (uint32_t i = 0; i < 3; i++) {
			VertexData v{};
			for (uint32_t j = 0; j < 3; j++) {
				end = indices[i].find_first_of('/');

				if (indices[i].find("//") == npos) {
					std::string sub = indices[i].substr(0, end);
					uint32_t value = std::stoi(sub);
					v[j] = value;
				}
				else {
					v[j] = 0;
				}

				indices[i].erase(0, end + 1);
			}

			face[i] = v;
		}
		outData.push_back(face);
	}
}

static vec3f getValue(Vec<vec3f>& list, uint64_t index) {
	if (index < list.size()) {
		return list.at(index);
	}
	else {
		return { 0.0f, 0.0f, 0.0f };
	}
}

void convertMeshData(Mesh* mesh, Vec<vec3f>& vertices, Vec<vec3f>& texCoords, Vec<vec3f>& normals, Vec<FaceData>& faces) {
	/*mesh->vertexCount = vertices.size();
	mesh->vertexData = new Vertex[mesh->vertexCount];*/
	mesh->indexCount = faces.size() * 3;
	mesh->indexData = new uint32_t[mesh->indexCount];
	
	std::unordered_map<Vertex, uint32_t> uniqueVertices{};
	std::vector<Vertex> vertexResults = {};
	std::vector<uint32_t> indices = {};

	//f 141/270/16 280/641/16 190/536/16
	uint32_t faceCount = faces.size();
	for (int i = 0; i < faceCount; i++) {
		FaceData currentFace = faces.at(i);
		// process 3 vertices for each face
		for (uint32_t j = 0; j < 3; j++) {
			VertexData currentVertex = currentFace[j];
			Vertex v = {};
			v.position = vertices.at(currentVertex.p - 1);
			v.texcoord = texCoords.at(currentVertex.t - 1);
			v.texcoord = { v.texcoord.x, 1.0f - v.texcoord.y, 0.0f, 0.0f };
			v.normal = normals.at(currentVertex.n - 1);

			if (uniqueVertices.count(v) == 0) {
				uniqueVertices[v] = vertexResults.size();
				vertexResults.push_back(v);
			}

			indices.push_back(uniqueVertices[v]);
			/*uint32_t currentVertexIndex = currentVertex.p - 1;
			uint32_t currentTexCoordIndex = currentVertex.t - 1;
			uint32_t currentNormalIndex = currentVertex.n - 1;

			vec3f v = vertices.at(currentVertexIndex);
			vec3f vt = texCoords.at(currentTexCoordIndex);
			vec3f n = normals.at(currentNormalIndex);

			mesh->indexData[(i * 3) + j] = currentVertexIndex;
			mesh->vertexData[currentVertexIndex].position = v;
			mesh->vertexData[currentVertexIndex].texcoord = shml::vec4f{ vt.x, vt.y, 0.0f, 0.0f };
			mesh->vertexData[currentVertexIndex].normal = n;*/
		}
	}

	mesh->vertexCount = vertexResults.size();
	mesh->vertexData = new Vertex[mesh->vertexCount];
	memcpy(mesh->vertexData, vertexResults.data(), sizeof(Vertex) * mesh->vertexCount);
	mesh->indexCount = indices.size();
	mesh->indexData = new uint32_t[mesh->indexCount];
	memcpy(mesh->indexData, indices.data(), sizeof(uint32_t) * mesh->indexCount);
}