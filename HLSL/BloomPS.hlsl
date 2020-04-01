struct PixelIn {
	float4 position : SV_POSITION;
	float2 texCoord;
};

[vk::binding(0, 0)]
SamplerState samplerState;

[vk::binding(1, 0)]
Texture2D tex;

[vk::constant_id(0)] const float criticalValue = 0.0f;

float4 main(PixelIn input) {
	float4 sampleColor = tex.Sample(samplerState, input.texCoord);

	const float3 W = float3(0.2125, 0.7154, 0.0721);
	float luminance = dot(sampleColor.rgb, W);

	luminance = saturate(luminance - criticalValue);

	sampleColor *= luminance;

	return sampleColor;
}