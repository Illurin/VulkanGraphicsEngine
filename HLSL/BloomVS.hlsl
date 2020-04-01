struct VertexOut {
	float4 position : SV_POSITION;
	float2 texCoord;
};

VertexOut main(uint vertexId : SV_VertexID) {
	VertexOut output;
	
	if (vertexId == 0) {
		output.position = float4(-1.0f, -1.0f, 0.0f, 1.0f);
		output.texCoord = float2(0.0f, 0.0f);
	}
	else if (vertexId == 1) {
		output.position = float4(1.0f, -1.0f, 0.0f, 1.0f);
		output.texCoord = float2(1.0f, 0.0f);
	}
	else if (vertexId == 2) {
		output.position = float4(-1.0f, 1.0f, 0.0f, 1.0f);
		output.texCoord = float2(0.0f, 1.0f);
	}
	else if (vertexId == 3) {
		output.position = float4(1.0f, 1.0f, 0.0f, 1.0f);
		output.texCoord = float2(1.0f, 1.0f);
	}

	return output;
}