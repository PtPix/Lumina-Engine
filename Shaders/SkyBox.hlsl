#include "Common.hlsli"

struct VSInput
{
    float3 Position : POSITION;
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
};

struct PSInput
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : TEXCOORD0;
};

PSInput VSMain(VSInput Input)
{
    PSInput Result;
    float4 pos = mul(mul(float4(Input.Position, 1.0f), ModelMatrix), ViewProj);
    Result.WorldPosition = Input.Position;
    Result.Position = pos.xyww;
    return Result;
}
float2 SampleSphericalMap(float3 v)
{
    float2 uv = float2(atan2(v.z, -v.x), asin(v.y));
    uv *= float2(0.1591f, 0.3183f);
    uv += 0.5f;
    return uv;
}

struct FBasePassOutput
{
    float4 SceneColor : SV_Target0; // RT0: Emissive
    float4 GBufferA : SV_Target1; // RT1: World Normal
    float4 GBufferB : SV_Target2; // RT2: Metallic, Specular, Roughness, Shading Model
    float4 GBufferC : SV_Target3; // RT3: BaseColor, AO
};

FBasePassOutput PSMain(PSInput Input)
{
    float3 dir = normalize(Input.WorldPosition);
    float2 uv = SampleSphericalMap(dir);

    float3 color = t_Skybox.Sample(gSamplerAniso, uv).rgb;

    color = color / (color + float3(1.0, 1.0, 1.0));
    color = pow(abs(color), float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    FBasePassOutput Output;
    Output.SceneColor = float4(color, 1.0f);
    Output.GBufferA = float4(0.0f, 0.0f, 0.0f, 0.0f);
    Output.GBufferB = float4(0.0f, 0.0f, 0.0f, 0.0f);
    Output.GBufferC = float4(0.0f, 0.0f, 0.0f, 0.0f);
    return Output;
}