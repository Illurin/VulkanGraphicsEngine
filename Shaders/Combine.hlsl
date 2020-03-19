struct PixelIn {
	float4 position : SV_POSITION;
	float2 texCoord;
};

[vk::binding(0, 0)]
SamplerState samplerState;

[vk::binding(1, 0)]
Texture2D tex;

float4 main(PixelIn input) {
	return tex.Sample(samplerState, input.texCoord);
}