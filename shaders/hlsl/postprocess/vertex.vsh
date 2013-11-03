uniform float BufferWidth;
uniform float BufferHeight;

struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

//======================================================================================
VS_OUTPUT main(VS_INPUT input)
{
	VS_OUTPUT output;
	float2 offset = float2(0.5/BufferWidth, 0.5/BufferHeight);
	output.Pos = float4(input.Pos.xy, 0.0, 1.0);
	output.Tex = input.Tex+offset;
	
	return output;
}
