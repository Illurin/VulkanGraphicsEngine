#include "Common.hlsl"

struct GeoIn {
	float3 center;
	float2 size;
	float4 color;
	float4 texCoord;
};

struct GeoOut {
	float4 posH : SV_POSITION;
	float2 texCoord;
	float4 color;
};

[maxvertexcount(4)]
void main(point GeoIn input, inout TriangleStream<GeoOut> stream) {
	float3 up = float3(0.0f, 1.0f, 0.0f);

	float3 look = eyePos - input.center;
	look = normalize(look);

	float3 right = cross(up, look);

	float halfWidth = 0.5f * input.size.x;
	float halfHeight = 0.5f * input.size.y;

	float4 v[4];

	v[0] = float4(input.center + halfWidth * right - halfHeight * up, 1.0f);
	v[1] = float4(input.center + halfWidth * right + halfHeight * up, 1.0f);
	v[2] = float4(input.center - halfWidth * right - halfHeight * up, 1.0f);
	v[3] = float4(input.center - halfWidth * right + halfHeight * up, 1.0f);

	float2 texCoord[4] = {
		float2(input.texCoord.x, input.texCoord.y + input.texCoord.w),
		float2(input.texCoord.x, input.texCoord.y),
		float2(input.texCoord.x + input.texCoord.z, input.texCoord.y + input.texCoord.w),
		float2(input.texCoord.x + input.texCoord.z, input.texCoord.y)
	};

	GeoOut output;
	float4x4 viewProjMatrix = mul(projMatrix, viewMatrix);

	[unroll]
	for (int i = 0; i < 4; i++) {
		output.posH = mul(viewProjMatrix, v[i]);
		output.texCoord = texCoord[i];
		output.color = input.color;

		stream.Append(output);
	}
}