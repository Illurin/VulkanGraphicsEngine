struct PixelIn {
	float4 position : SV_POSITION;
	float2 texCoord;
};

[vk::binding(0, 0)]
SamplerState samplerState;

[vk::binding(0, 0)]
Texture2D tex;

[vk::constant_id(0)] const float offset = 0.0f;
[vk::constant_id(1)] const int radius = 0;

float4 main(PixelIn input) {
	const float weights[5] = { 0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216 };

	uint width, height, numMips;
	tex.GetDimensions(0, width, height, numMips);

	float texOffset = 1.0f / (float)height;

	float3 blurColor = tex.Sample(samplerState, input.texCoord).rgb * weights[0];

	int sampleCount = 5;

	for (int i = 1; i < sampleCount; i++) {
		blurColor += tex.Sample(samplerState, input.texCoord + float2(0.0f, texOffset * (float)i)).rgb * weights[i];
		blurColor += tex.Sample(samplerState, input.texCoord - float2(0.0f, texOffset * (float)i)).rgb * weights[i];
	}

	return float4(blurColor, 1.0f);
}