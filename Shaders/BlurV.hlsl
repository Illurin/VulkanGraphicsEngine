struct PixelIn {
	float4 position : SV_POSITION;
	float2 texCoord;
};

[vk::binding(0, 0)]
SamplerState samplerState;

[vk::binding(1, 0)]
Texture2D tex;

float4 main(PixelIn input) {
	float4 blurColor = float4(0.0f, 0.0f, 0.0f, 0.0f);
	
	int radius = 10;

	int sampleCount = radius * 2 + 1;
	float offset = 0.003f;

	for (int i = 0; i < sampleCount; i++) {
		float2 texCoord = input.texCoord + float2(0.0f, -offset * (float)radius  + offset * (float)i);
		blurColor += tex.Sample(samplerState, texCoord);
	}

	blurColor /= (float)sampleCount;

	return blurColor;
}