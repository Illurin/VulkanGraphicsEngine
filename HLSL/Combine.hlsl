struct PixelIn {
	float4 position : SV_POSITION;
	float2 texCoord;
};

[vk::binding(0, 0)]
cbuffer HDRProperties {
	float exposure;
};

[vk::binding(1, 0)] Texture2D source0;
[vk::binding(1, 0)] SamplerState sourceSampler;
[vk::binding(2, 0)] Texture2D source1;

float4 main(PixelIn input) {
	float3 hdrColor = source0.Sample(sourceSampler, input.texCoord).rgb + source1.Sample(sourceSampler, input.texCoord).rgb;
	hdrColor = float3(1.0f) - exp(-hdrColor * exposure);

	return float4(hdrColor, 1.0f);
}