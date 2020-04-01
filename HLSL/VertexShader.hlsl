#include "Common.hlsl"

struct VertexIn {
	float3 posL;
	float2 texCoord;
	float3 normal;
	float3 tangent;
};

struct VertexOut {
	float4 position : SV_POSITION;
	float3 posW;
	float2 texCoord;
	float3 normal;
	float3 tangent;
	float4 shadowPos;
};

[vk::binding(0, 0)]
cbuffer ObjectConstants {
	float4x4 worldMatrix;
	float4x4 worldMatrix_trans_inv;
};

VertexOut main(VertexIn input) {
	VertexOut output;

	float4x4 viewProjMatrix = mul(projMatrix, viewMatrix);

	output.posW = float3(mul(worldMatrix, float4(input.posL, 1.0f)));
	output.position = mul(viewProjMatrix, float4(output.posW, 1.0f));
	output.normal = float3(mul(worldMatrix_trans_inv, float4(input.normal, 0.0f)));
	output.tangent = float3(mul(worldMatrix, float4(input.tangent, 0.0f)));

	output.shadowPos = mul(shadowTransform, float4(output.posW, 1.0f));
	output.texCoord = input.texCoord;

	return output;
}