#include "Common.hlsl"

struct Material {
	float4 diffuseAlbedo;
	float3 fresnelR0;
	float shininess;
};

/*计算衰减参数*/
float CalcAttenuation(float distance, float fallOffStart, float fallOffEnd) {
	//线性衰减
	return saturate((fallOffEnd - distance) / (fallOffEnd - fallOffStart));
}

/*Schlick近似法计算Fresnel方程*/
float3 SchlickFresnel(float3 R0, float3 normal, float3 lightVec) {
	//计算法向量和光向量的夹角余弦值
	float cosIncidentAngle = saturate(dot(normal, lightVec));
	//Schlick近似方程
	float f0 = 1.0f - cosIncidentAngle;
	float3 reflectPercent = R0 + (1.0f - R0) * pow(f0, 5);
	return reflectPercent;
}

/*冯氏光照模型*/
float3 BlinnPhong(float3 lightStrength, float3 lightVec, float3 normal, float3 toEye, Material mat) {
	const float shininess = mat.shininess * 256.0f;
	//为计算表面粗糙度 计算半角向量
	float3 halfVec = normalize(toEye + lightVec);
	//计算出表面粗糙度
	float roughnessFactor = (shininess + 8.0f) * pow(max(dot(halfVec, normal), 0.0f), shininess) / 8.0f;
	//Fresnel因子(半角向量作为法向量)
	float3 fresnelFactor = SchlickFresnel(mat.fresnelR0, halfVec, lightVec);
	//计算出镜面反射值
	float3 specAlbedo = fresnelFactor * roughnessFactor;
	//漫反射 + 镜面反射 * 光强
	return (mat.diffuseAlbedo.rgb + specAlbedo) * lightStrength;
}

/*方向光计算函数*/
float3 ComputeDirectionalLight(Light light, Material mat, float3 normal, float3 toEye) {
	float3 lightVec = -light.direction;
	//用Lambert余弦定理缩小光强
	float lambertFactor = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = light.strength * lambertFactor;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

/*点光源计算函数*/
float3 ComputePointLight(Light light, Material mat, float3 pos, float3 normal, float3 toEye) {
	float3 lightVec = light.position - pos;

	//物体和光源的距离
	float distance = length(lightVec);
	if (distance > light.fallOffEnd)
		return 0.0f; //超出衰减范围，不接收光照
	lightVec /= distance;

	//用Lambert余弦定理缩小光强
	float lambertFactor = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = light.strength * lambertFactor;

	//计算线性衰减
	float att = CalcAttenuation(distance, light.fallOffStart, light.fallOffEnd);
	lightStrength *= att;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

/*聚光灯计算函数*/
float3 ComputeSpotLight(Light light, Material mat, float3 pos, float3 normal, float3 toEye) {
	float3 lightVec = light.position - pos;

	//物体和光源的距离
	float distance = length(lightVec);
	if (distance > light.fallOffEnd)
		return 0.0f; //超出衰减范围，不接收光照
	lightVec /= distance;

	//用Lambert余弦定理缩小光强
	float lambertFactor = max(dot(lightVec, normal), 0.0f);
	float3 lightStrength = light.strength * lambertFactor;

	//计算线性衰减
	float att = CalcAttenuation(distance, light.fallOffStart, light.fallOffEnd);
	lightStrength *= att;

	//计算聚光灯衰减
	float spotFactor = pow(max(dot(-lightVec, light.direction), 0.0f), light.spotPower);
	lightStrength *= spotFactor;

	return BlinnPhong(lightStrength, lightVec, normal, toEye, mat);
}

//阴影贴图
[vk::binding(0, 3)]
SamplerComparisonState shadowSampler;

[vk::binding(1, 3)]
Texture2D shadowMap;

//计算PCF
float CalcShadowFactor(float4 shadowPos) {
	shadowPos.xyz /= shadowPos.w;

	float depth = shadowPos.z;
	uint width, height, numMips;
	shadowMap.GetDimensions(0, width, height, numMips);
	float dx = 1.0f / (float)width;
	float percentLit = 0.0f;

	const float2 offsets[9] = {
		float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
		float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
		float2(-dx, dx), float2(0, dx), float2(dx, dx)
	};

	for (int i = 0; i < 9; i++)
		percentLit += shadowMap.SampleCmpLevelZero(shadowSampler, shadowPos.xy + offsets[i], depth).r;
	return percentLit / 9.0f;
}