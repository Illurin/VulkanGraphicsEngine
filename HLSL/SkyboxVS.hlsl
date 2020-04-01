struct Light {
	float3 strength;
	float fallOffStart;					   //point/spot light only
	float3 direction;					   //directional/spot light only
	float fallOffEnd;					   //point/spot light only
	float3 position;					   //point/spot light only
	float spotPower;					   //spot light only
};

[vk::binding(0, 1)]
cbuffer PassConstants {
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4x4 shadowTransform;

	float4 eyePos;
	float4 ambientLight;

	Light lights[3];
};

struct VertexOut {
	float4 position : SV_POSITION;
	float3 posL;
};

struct VertexIn {
	float3 posH;
	float2 texCoord;
	float3 normal;
	float3 tangent;
};

VertexOut main(VertexIn input)
{
	VertexOut output;

	//用局部顶点的位置作为立方体图的查找向量
	output.posL = input.posH;

	//把顶点变换到齐次裁剪空间
	float4 posW = float4(input.posH, 1.0f);

	//总是以摄像机作为天空球的中心
	posW.xyz += eyePos;

	float4x4 viewProjMatrix = mul(projMatrix, viewMatrix);
	posW = mul(viewProjMatrix, posW);

	//使z = w 令球面总是位于远平面
	output.position = posW.xyww;

	return output;
}