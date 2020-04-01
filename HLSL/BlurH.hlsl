struct PixelIn {
	float4 position : SV_POSITION;
	float2 texCoord;
};

[vk::binding(0, 0)]
SamplerState samplerState;

[vk::binding(1, 0)]
Texture2D tex;

[vk::constant_id(0)] const float offset = 0.0f;
[vk::constant_id(1)] const int radius = 0;

float4 main(PixelIn input) {
	float4 blurColor = float4(0.0f, 0.0f, 0.0f, 0.0f);

	int sampleCount = radius * 2 + 1;

	for (int i = 0; i < sampleCount; i++) {
		float2 texCoord = input.texCoord + float2(-offset * (float)radius + offset * (float)i, 0.0f);
		blurColor += tex.Sample(samplerState, texCoord);
	}

	blurColor /= (float)sampleCount;

	return blurColor;
}