#include "LightingUtil.hlsl"

[vk::input_attachment_index(0)][vk::binding(0, 1)] SubpassInput inDiffuseAlbedo;
[vk::input_attachment_index(1)][vk::binding(1, 1)] SubpassInput inNormal;
[vk::input_attachment_index(2)][vk::binding(2, 1)] SubpassInput inMaterialProperties;
[vk::input_attachment_index(3)][vk::binding(3, 1)] SubpassInput inPosition;
[vk::input_attachment_index(4)][vk::binding(4, 1)] SubpassInput inShadowPos;

struct PixelIn {
	float4 position : SV_POSITION;
	float2 texCoord;
};

float4 main(PixelIn input) : SV_TARGET{
	//从InputAttachment中读出所有数据
	float4 diffuse = inDiffuseAlbedo.SubpassLoad();
	float3 posW = inPosition.SubpassLoad().rgb;
	float3 normal = inNormal.SubpassLoad().rgb;
	float4 shadowPos = inShadowPos.SubpassLoad();
	float3 fresnelR0 = inMaterialProperties.SubpassLoad().rgb;
	float roughness = inMaterialProperties.SubpassLoad().a;

	//计算出光照的基本参数
	float3 toEye = normalize(eyePos.xyz - posW);

	//整合材质
	const float shiniess = 1.0f - roughness;
	Material mat = { float4(1.0f, 1.0f, 1.0f, 1.0f), fresnelR0, shiniess };

	//计算出阴影因子
	float shadowFactor = CalcShadowFactor(shadowPos);

	//计算出光照的值
	float3 lightingResult = float3(0.0f, 0.0f, 0.0f);

	[unroll]
	for (int i = 0; i < NUM_DIRECTIONAL_LIGHT; i++)
		lightingResult += ComputeDirectionalLight(lights[i], mat, normal, toEye);

	[unroll]
	for (int i = 0; i < NUM_POINT_LIGHT; i++) {
		int lightIndex = NUM_DIRECTIONAL_LIGHT + i;
		lightingResult += ComputePointLight(lights[lightIndex], mat, posW, normal, toEye);
	}

	[unroll]
	for (int i = 0; i < NUM_SPOT_LIGHT; i++) {
		int lightIndex = NUM_DIRECTIONAL_LIGHT + NUM_POINT_LIGHT + i;
		lightingResult += ComputeSpotLight(lights[lightIndex], mat, posW, normal, toEye);
	}

	lightingResult *= shadowFactor;

	//计算出最终的颜色结果
	float4 litColor = diffuse * (ambientLight + float4(lightingResult, 1.0f));
	litColor.a = 1.0f;

	return litColor;
}