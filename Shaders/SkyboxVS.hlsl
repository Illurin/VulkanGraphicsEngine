#include "Common.hlsl"

struct VertexOut {
	float4 position : SV_POSITION;
	float3 posL;
};

struct VertexIn {
	float3 posH;
	float2 texCoord;
	float3 normal;
};

[vk::binding(0, 0)]
cbuffer ObjectConstants {
	float4x4 worldMatrix;
};

VertexOut main(VertexIn input)
{
	VertexOut output;

	//用局部顶点的位置作为立方体图的查找向量
	output.posL = input.posH;

	//把顶点变换到齐次裁剪空间
	float4 posW = mul(worldMatrix, float4(input.posH, 1.0f));

	//总是以摄像机作为天空球的中心
	posW.xyz += eyePos;

	float4x4 viewProjMatrix = mul(projMatrix, viewMatrix);
	posW = mul(viewProjMatrix, posW);

	//使z = w 令球面总是位于远平面
	output.position = posW.xyww;

	return output;
}