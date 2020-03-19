#pragma once
#include "vkUtil.h"

class GeometryGenerator {
public:
	struct MeshData {
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;

		MeshData(){}
		MeshData(std::vector<Vertex>& vertices, std::vector<uint32_t>& indices) {
			this->vertices.insert(this->vertices.end(), vertices.begin(), vertices.end());
			this->indices.insert(this->indices.end(), indices.begin(), indices.end());
		}
	};

	MeshData CreatePlane(float width, float depth, uint32_t texRepeatX, uint32_t texRepeatY);
	MeshData CreateGeosphere(float radius, uint32_t numSubdivisions);

	void Subdivide(MeshData& meshData);
	Vertex GetMidPoint(const Vertex& v0, const Vertex& v1);
};