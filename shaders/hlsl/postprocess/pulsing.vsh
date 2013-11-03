// nvidia shader library
// http://developer.download.nvidia.com/shaderlibrary/webpages/shader_library.html

uniform float BufferWidth;
uniform float BufferHeight;
uniform float Speed;
uniform float Speed2;
uniform float Pulse;
uniform float PulseE;
uniform float CenterX;
uniform float CenterY;
uniform float ElapsedTime;

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
    output.Pos = input.Pos;
    
    float r = ElapsedTime*Speed;
    float2 cs = float2(sin(r), cos(r));
    r = 2.0*(pow(0.5*(sin(Speed2*ElapsedTime)+1.0), PulseE)-0.5);
    r = 1 + Pulse*r;
    
    float2 ctr = float2(CenterX, CenterY);
    float2 t = r*(float2(input.Tex.xy + offset)-ctr);
    output.Tex = (((float2((t.x*cs.x - t.y*cs.y),(t.x*cs.y + t.y*cs.x))))+ctr);

    return output;
}