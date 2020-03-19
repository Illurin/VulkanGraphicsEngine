#include "Common.hlsl"

struct PixelIn {
	float4 position : SV_POSITION;
};

[vk::binding(2, 0)]
cbuffer MaterialConstants {
	float4x4 matTransform;
	float4 diffuseAlbedo;
	float3 fresnelR0;
	float roughness;
};

[vk::binding(1, 1)]
SamplerState samplerState;

[vk::binding(1, 0)]
Texture2D tex;

void main(PixelIn input) {
	//do nothing
}