#include "LightingUtil.hlsl"

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
};

[vk::constant_id(0)] const int shaderModel = 0;

[vk::binding(1, 1)]
SamplerState samplerState;

[vk::binding(2, 1)]
Texture2D tex;

[vk::binding(3, 1)]
Texture2D normalMap;

float3 NormalSampleToWorldSpace(float3 normalMapSample, float3 unitNormalW, float3 tangentW) {
	//映射法向量从[0,1]至[-1,1]
	float3 normalT = 2.0f * normalMapSample - 1.0f;

	//构建TBN基底
	float3 N = unitNormalW;
	float3 T = normalize(tangentW - dot(tangentW, N) * N);
	float3 B = cross(N, T);

	float3x3 TBN = float3x3(T, B, N);

	//将法向量从切线空间变换至世界空间
	return mul(normalT, TBN);
}

float4 main(PixelIn input) : SV_TARGET
{
	//采样得到基础色
	float4 sampleColor = tex.Sample(samplerState, mul((float2x2)matTransform, input.texCoord));

	//得到归一化法线
	input.normal = normalize(input.normal);
	if (shaderModel == 1) {
		float4 normalMapSample = normalMap.Sample(samplerState, mul((float2x2)matTransform, input.texCoord));
		input.normal = NormalSampleToWorldSpace(normalMapSample.rgb, input.normal, input.tangent);
	}

	//计算出光照的基本参数
	float3 toEye = normalize(eyePos.xyz - input.posW);

	//计算出漫反射色
	float4 diffuse = sampleColor * diffuseAlbedo;

	//整合材质
	const float shiniess = 1.0f - roughness;
	Material mat = { diffuseAlbedo, fresnelR0, shiniess };

	//计算出阴影因子
	float shadowFactor = CalcShadowFactor(input.shadowPos);

	//计算出光照的值
	float3 lightingResult = shadowFactor * (ComputeDirectionalLight(lights[0], mat, input.normal, toEye)
		+ ComputePointLight(lights[NUM_DIRECTIONAL_LIGHT], mat, input.posW, input.normal, toEye)
		+ ComputeSpotLight(lights[NUM_DIRECTIONAL_LIGHT + NUM_POINT_LIGHT], mat, input.posW, input.normal, toEye));

	//计算出最终的颜色结果
	float4 litColor = diffuse * (ambientLight + float4(lightingResult, 1.0f));
	litColor.a = sampleColor.a;

	return litColor;
}