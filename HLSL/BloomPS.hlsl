struct PixelIn {
	float4 position : SV_POSITION;
	float2 texCoord;
};

[vk::binding(0, 0)] Texture2D sourceImage;
[vk::binding(0, 0)] SamplerState sourceImageSampler;

[vk::constant_id(0)] const float criticalValue = 0.0f;

float4 main(PixelIn input) {
	float4 sampleColor = sourceImage.Sample(sourceImageSampler, input.texCoord);

	const float3 W = float3(0.2125, 0.7154, 0.0721);
	float luminance = dot(sampleColor.rgb, W);

	float3 brightColor = float3(0.0f);

	if (luminance > criticalValue) {
		brightColor = sampleColor.rgb;
	}

	return float4(brightColor, 1.0f);
}