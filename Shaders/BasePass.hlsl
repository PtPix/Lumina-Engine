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
    float2 UV : TEXCOORD;
    float3 Normal : NORMAL;
    float3 WorldPosition : POSITION;
};

PSInput VSMain(VSInput Input)
{
    PSInput Result;
    Result.Position = mul(mul(float4(Input.Position, 1.0f), ModelMatrix), ViewProj);
    Result.UV = Input.UV;
    Result.Normal = mul(Input.Normal, (float3x3)ModelMatrix);
    Result.WorldPosition = mul(float4(Input.Position, 1.0f), ModelMatrix).xyz;
    return Result;
}

float3 PerturbNormal(float3 worldPos, float3 surfNormal, float2 uv, float3 normalMap)
{
    float3 q1 = ddx(worldPos);
    float3 q2 = ddy(worldPos);
    float2 st1 = ddx(uv);
    float2 st2 = ddy(uv);

    float3 N = normalize(surfNormal);
    float3 T = normalize(q1 * st2.y - q2 * st1.y);
    float3 B = -normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);

    normalMap = normalMap * 2.0 - 1.0;
    return normalize(mul(normalMap, TBN));
}

struct FBasePassOutput
{
    float4 SceneColor : SV_Target0; // RT0: Emissive
    float4 GBufferA : SV_Target1; // RT1: World Normal
    float4 GBufferB : SV_Target2; // RT2: Metallic, Specular, Roughness, Shading Model
    float4 GBufferC : SV_Target3; // RT3: BaseColor, AO
};

#define SHADINGMODELID_DEFAULT_LIT 1
#define SHADINGMODELID_UNLIT 2

FBasePassOutput PSMain(PSInput Input)
{
    // BaseColor Gamma Correction
    float4 BaseColorTextureValue = t_Albedo.Sample(gSamplerAniso, Input.UV);
    float3 BaseColor = pow(BaseColorTextureValue.xyz, float3(2.2, 2.2, 2.2));

    // Normal
    float3 NormalMap = t_Normal.Sample(gSamplerAniso, Input.UV).rgb;
    float3 WorldNormal = PerturbNormal(Input.WorldPosition.xyz, Input.Normal, Input.UV, NormalMap);
    float3 FinalWorldNormal = WorldNormal * 0.5f + 0.5f;

    // Metal Roughness Specular
    float4 MetalRoughnessTextureValue = t_MetalRough.Sample(gSamplerAniso, Input.UV);
    float FinalMetallic = MetalRoughnessTextureValue.b * Metallic;
    float FinalRoughness = max(MetalRoughnessTextureValue.g * Roughness, 0.04f);
    float FinalSpecular = 0.5f;

    // Emissive and gamma correction
    float3 EmissiveTextureValue = t_Emissive.Sample(gSamplerAniso, Input.UV).rgb;
    float3 FinalEmissive = pow(abs(EmissiveTextureValue), float3(2.2, 2.2, 2.2));

    // AO
    float FinalAO = t_AO.Sample(gSamplerAniso, Input.UV).r * AO;

    FBasePassOutput Output;
    Output.SceneColor = float4(FinalEmissive, 1.0f);
    Output.GBufferA = float4(FinalWorldNormal, 1.0f);
    float ShadingModel = (float)SHADINGMODELID_DEFAULT_LIT / 255.0f;
    Output.GBufferB = float4(FinalMetallic, FinalSpecular, FinalRoughness, ShadingModel);
    Output.GBufferC = float4(BaseColor, FinalAO);

    return Output;
}