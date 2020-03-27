#include "GeometryGenerator.h"

GeometryGenerator::MeshData GeometryGenerator::CreatePlane(float width, float depth, uint32_t texRepeatX, uint32_t texRepeatY) {
	float halfWidth = 0.5f * width;
	float halfDepth = 0.5f * depth;
	float texCoordX = 1.0f * static_cast<float>(texRepeatX);
	float texCoordY = 1.0f * static_cast<float>(texRepeatY);

	std::vector<Vertex> vertices(4);
	vertices[0].position = { -halfWidth, 0, -halfDepth };
	vertices[1].position = { halfWidth, 0, -halfDepth };
	vertices[2].position = { -halfWidth, 0, halfDepth };
	vertices[3].position = { halfWidth, 0, halfDepth };
	vertices[0].texCoord = { 0.0f, 0.0f };
	vertices[1].texCoord = { texCoordX, 0.0f };
	vertices[2].texCoord = { 0.0f, texCoordY };
	vertices[3].texCoord = { texCoordX, texCoordY };
	vertices[0].normal = { 0.0f, 1.0f, 0.0f };
	vertices[1].normal = { 0.0f, 1.0f, 0.0f };
	vertices[2].normal = { 0.0f, 1.0f, 0.0f };
	vertices[3].normal = { 0.0f, 1.0f, 0.0f };
	vertices[0].tangent = { 1.0f, 0.0f, 0.0f };
	vertices[1].tangent = { 1.0f, 0.0f, 0.0f };
	vertices[2].tangent = { 1.0f, 0.0f, 0.0f };
	vertices[3].tangent = { 1.0f, 0.0f, 0.0f };

	std::vector<UINT> indices = {
		0, 1, 3, 0, 3, 2
	};

	return MeshData(vertices, indices);
}

GeometryGenerator::MeshData GeometryGenerator::CreateGeosphere(float radius, uint32_t numSubdivisions) {
	MeshData meshData;
	numSubdivisions = (numSubdivisions > 6) ? 6 : numSubdivisions;

	//对一个正二十面体进行曲面细分
	const float X = 0.525731f;
	const float Z = 0.850651f;
	std::vector<glm::vec3> pos = {
		glm::vec3(-X, 0.0f, Z),  glm::vec3(X, 0.0f, Z),
		glm::vec3(-X, 0.0f, -Z), glm::vec3(X, 0.0f, -Z),
		glm::vec3(0.0f, Z, X),   glm::vec3(0.0f, Z, -X),
		glm::vec3(0.0f, -Z, X),  glm::vec3(0.0f, -Z, -X),
		glm::vec3(Z, X, 0.0f),   glm::vec3(-Z, X, 0.0f),
		glm::vec3(Z, -X, 0.0f),  glm::vec3(-Z, -X, 0.0f)
	};
	std::vector<UINT> indices = {
		1,4,0,  4,9,0,  4,5,9,  8,5,4,  1,8,4,
		1,10,8, 10,3,8, 8,3,5,  3,2,5,  3,7,2,
		3,10,7, 10,6,7, 6,11,7, 6,0,11, 6,1,0,
		10,1,6, 11,0,9, 2,11,9, 5,2,9,  11,2,7
	};

	meshData.vertices.resize(12);
	meshData.indices.insert(meshData.indices.end(), indices.begin(), indices.end());

	//细分
	for (UINT i = 0; i < 12; i++)
		meshData.vertices[i].position = pos[i];
	for (UINT i = 0; i < numSubdivisions; i++)
		Subdivide(meshData);

	//为每个顶点计算纹理坐标
	for (UINT i = 0; i < meshData.vertices.size(); i++)
	{
		//将每个顶点投射到球面上
		glm::vec3 n = glm::normalize(meshData.vertices[i].position);
		glm::vec3 p = n * radius;
		
		meshData.vertices[i].position = p;
		meshData.vertices[i].normal = n;

		//根据球面坐标推导纹理坐标
		float theta = atan2f/*反正切*/(meshData.vertices[i].position.z, meshData.vertices[i].position.x);
		if (theta < 0.0f)
			theta += glm::pi<float>();
		float phi = acosf/*反余弦*/(meshData.vertices[i].position.y / radius);

		meshData.vertices[i].texCoord.x = theta / glm::two_pi<float>();
		meshData.vertices[i].texCoord.y = phi / glm::pi<float>();

		//计算出切线
		meshData.vertices[i].tangent.x = -radius * sinf(phi) * sinf(theta);
		meshData.vertices[i].tangent.y = 0.0f;
		meshData.vertices[i].tangent.z = +radius * sinf(phi) * cosf(theta);

		meshData.vertices[i].tangent = glm::normalize(meshData.vertices[i].tangent);
	}
	return meshData;
}

void GeometryGenerator::Subdivide(MeshData& meshData) {
	MeshData inputCopy = meshData;

	meshData.vertices.resize(0);
	meshData.indices.resize(0);

	//       v1
	//       *
	//      / \
	//     /   \
	//  m0*-----*m1
	//   / \   / \
	//  /   \ /   \
	// *-----*-----*
	// v0    m2     v2

	UINT numTris = (UINT)inputCopy.indices.size() / 3;
	for (int i = 0; i < numTris; i++)
	{
		Vertex v0 = inputCopy.vertices[inputCopy.indices[i * 3]];
		Vertex v1 = inputCopy.vertices[inputCopy.indices[i * 3 + 1]];
		Vertex v2 = inputCopy.vertices[inputCopy.indices[i * 3 + 2]];

		//生成三个顶点的中点
		Vertex m0 = GetMidPoint(v0, v1);
		Vertex m1 = GetMidPoint(v1, v2);
		Vertex m2 = GetMidPoint(v0, v2);

		//添加新的几何体
		meshData.vertices.push_back(v0);		//0
		meshData.vertices.push_back(v1);		//1
		meshData.vertices.push_back(v2);		//2
		meshData.vertices.push_back(m0);		//3
		meshData.vertices.push_back(m1);		//4
		meshData.vertices.push_back(m2);		//5

		meshData.indices.push_back(i * 6 + 0);
		meshData.indices.push_back(i * 6 + 3);
		meshData.indices.push_back(i * 6 + 5);

		meshData.indices.push_back(i * 6 + 3);
		meshData.indices.push_back(i * 6 + 4);
		meshData.indices.push_back(i * 6 + 5);

		meshData.indices.push_back(i * 6 + 5);
		meshData.indices.push_back(i * 6 + 4);
		meshData.indices.push_back(i * 6 + 2);

		meshData.indices.push_back(i * 6 + 3);
		meshData.indices.push_back(i * 6 + 1);
		meshData.indices.push_back(i * 6 + 4);
	}
}

Vertex GeometryGenerator::GetMidPoint(const Vertex& v0, const Vertex& v1) {
	glm::vec3 p0 = v0.position;
	glm::vec3 p1 = v1.position;

	glm::vec3 n0 = v0.normal;
	glm::vec3 n1 = v1.normal;

	glm::vec3 t0 = v0.tangent;
	glm::vec3 t1 = v1.tangent;

	glm::vec2 tex0 = v0.texCoord;
	glm::vec2 tex1 = v1.texCoord;

	glm::vec3 position = (p0 + p1) * 0.5f;
	glm::vec3 normal = (n0 + n1) * 0.5f;
	glm::vec3 tangent = (t0 + t1) * 0.5f;
	glm::vec2 texCoord = (tex0 + tex1) * 0.5f;

	Vertex vertex;
	vertex.position = position;
	vertex.normal = normal;
	vertex.tangent = tangent;
	vertex.texCoord = texCoord;
	return vertex;
}