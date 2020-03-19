#pragma once
#include "vkUtil.h"

struct MeshRenderer {
	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
};

struct SkinnedMeshRenderer {
	std::vector<SkinnedVertex> vertices;
	std::vector<uint32_t> indices;
};
