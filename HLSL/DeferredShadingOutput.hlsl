struct PixelIn {
	float4 position : SV_POSITION;
	float3 posW;
	float2 texCoord;
	float3 normal;
	float3 tangent;
	float4 shadowPos;
};

[vk::binding(0, 1)]
cbuffer MaterialConstants {
	float4x4 matTransform;
	float4 diffuseAlbedo;
	float3 fresnelR0;
	float roughness;
}

[vk::binding(1, 1)]
SamplerState samplerState;

[vk::binding(2, 1)]
Texture2D tex;

[vk::binding(3, 1)]
Texture2D normalMap;

[vk::constant_id(0)] const int shaderModel = 0;

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW) {
	//映射法向量从[0,1]至[-1,1]
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	//构建TBN基底
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	//将法向量从切线空间变换至世界空间
	float3 normal = mul(normalT, TBN);
	normal.z = -normal.z;

	return normal;
}

void main(PixelIn input,
	[vk::location(0)] out float4 diffuse,
	[vk::location(1)] out float4 normal,
	[vk::location(2)] out float4 materialProperties,
	[vk::location(3)] out float4 position,
	[vk::location(4)] out float4 shadowPos) {
	diffuse = diffuseAlbedo * tex.Sample(samplerState, mul((float2x2)matTransform, input.texCoord));

	normal.xyz = normalize(input.normal);
	if (shaderModel == 1) {
		float4 normalMapSample = normalMap.Sample(samplerState, mul((float2x2)matTransform, input.texCoord));
		normal.xyz = NormalSampleToWorldSpace(normalMapSample.rgb, input.normal, input.tangent);
	}

	materialProperties = float4(fresnelR0, roughness);
	position = float4(input.posW, 1.0f);
	shadowPos = input.shadowPos;
}