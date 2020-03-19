struct PixelIn {
	float4 posH : SV_POSITION;
	float2 texCoord;
	float4 color;
};

[vk::binding(1, 1)]
SamplerState samplerState;

[vk::binding(1, 0)]
Texture2D tex;

float4 main(PixelIn input) : SV_TARGET{
	float4 sampleColor = tex.Sample(samplerState, input.texCoord);
	sampleColor.rgb += input.color.rgb;
	sampleColor.a *= input.color.a;
	return sampleColor;
}