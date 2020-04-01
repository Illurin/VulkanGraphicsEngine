#include "Common.hlsl"

struct VertexIn {
	float3 posL;
	float size;
	float4 color;
	float4 texCoord;
};

struct VertexOut {
	float3 center;
	float2 size;
	float4 color;
	float4 texCoord;
};

[vk::binding(0, 0)]
cbuffer ObjectConstants {
	float4x4 worldMatrix;
	float4x4 worldMatrix_trans_inv;
};

VertexOut main(VertexIn input) {
	VertexOut output;

	output.center = float3(mul(worldMatrix, float4(input.posL, 1.0f)));
	output.size = float2(input.size, input.size);
	output.color = input.color;
	output.texCoord = input.texCoord;

	return output;
}