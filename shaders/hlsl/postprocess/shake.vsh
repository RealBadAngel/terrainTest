// nvidia shader library
// http://developer.download.nvidia.com/shaderlibrary/webpages/shader_library.html

uniform float BufferWidth;
uniform float BufferHeight;
uniform float Sharpness;
uniform float Shake;
uniform float RandomValue;

struct VS_INPUT
{
	float4 Pos:POSITION;
	float2 Tex:TEXCOORD0;
};

struct VS_OUTPUT
{
	float4 Pos:POSITION;
	float2 Tex:TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
	float2 offset = float2(0.5/BufferWidth, 0.5/BufferHeight);
  
	VS_OUTPUT output;
    output.Pos = float4(input.Pos.xy, 0.0, 1.0);
    float2 i = Shake*float2(RandomValue, RandomValue);
    float2 j = abs(i);
    float2 pj = pow(j, Sharpness);
    float2 si = sign(i);
    float2 shook = si * pj;
	output.Tex = input.Tex.xy+shook+offset;
    return output;
}