#include "Common.hlsl"

struct VertexOut {
	float4 position : SV_POSITION;
	float3 posW;
	float2 texCoord;
	float3 normal;
	float3 tangent;
	float4 shadowPos;
};

struct VertexIn {
	float3 position;
	float2 texCoord;
	float3 normal;
	float3 tangent;

	float3 boneWeights;
	uint4 boneIndices;
};

[vk::binding(0, 0)]
cbuffer ObjectConstants {
	float4x4 worldMatrix;
	float4x4 worldMatrix_trans_inv;
};

[vk::binding(0, 4)]
cbuffer SkinnedConstants {
	float4x4 boneTransforms[500];
	float4x4 boneTransforms_inv_trans[500];
};

VertexOut main(VertexIn input)
{
	VertexOut output;

	float3 posL = float3(0.0f, 0.0f, 0.0f);    //Local Space
	float3 normalL = float3(0.0f, 0.0f, 0.0f);
	float3 tangentL = float3(0.0f, 0.0f, 0.0f);

	/*Bone Animation*/
	float weights[4] = { input.boneWeights.x, input.boneWeights.y, input.boneWeights.z, 1.0f - input.boneWeights.x - input.boneWeights.y - input.boneWeights.z };

	for (int i = 0; i < 4; i++) {
		posL += weights[i] * mul(boneTransforms[input.boneIndices[i]], float4(input.position, 1.0f)).xyz;
		normalL += weights[i] * mul((float3x3)boneTransforms_inv_trans[input.boneIndices[i]], input.normal);
		normalL += weights[i] * mul((float3x3)boneTransforms[input.boneIndices[i]], input.normal);
	}

	float4 transformPosition = mul(worldMatrix, float4(posL, 1.0f));
	float4x4 viewProjMatrix = mul(projMatrix, viewMatrix);

	output.posW = transformPosition.xyz;
	output.position = mul(viewProjMatrix, transformPosition);
	output.normal = mul((float3x3)worldMatrix_trans_inv, normalL);
	output.tangent = mul((float3x3)worldMatrix, tangentL);
	output.texCoord = input.texCoord;
	output.shadowPos = mul(shadowTransform, transformPosition);

	return output;
}