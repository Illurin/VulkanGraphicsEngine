#define NUM_DIRECTIONAL_LIGHT 1
#define NUM_POINT_LIGHT 10
#define NUM_SPOT_LIGHT 1

struct Light {
	float3 strength;
	float fallOffStart;					   //point/spot light only
	float3 direction;					   //directional/spot light only
	float fallOffEnd;					   //point/spot light only
	float3 position;					   //point/spot light only
	float spotPower;					   //spot light only
};

[vk::binding(0, 2)]
cbuffer PassConstants {
	float4x4 viewMatrix;
	float4x4 projMatrix;
	float4x4 shadowTransform;

	float4 eyePos;
	float4 ambientLight;

	Light lights[NUM_DIRECTIONAL_LIGHT + NUM_POINT_LIGHT + NUM_SPOT_LIGHT];
};