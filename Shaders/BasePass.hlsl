// Assets/Shaders/BasePass.hlsl

// ==========================================
// 1. 数据结构 (严格对齐 C++ 端的 16 字节)
// ==========================================
struct InstanceData
{
    float4x4 WorldMatrix;
    uint     MaterialIndex;
    uint3    Pad;
};

struct FPBRMaterialData
{
    float4 BaseColorFactor;
    float4 EmissiveFactor;

    float MetallicFactor;
    float RoughnessFactor;
    float AlphaCutoff;
    uint  ShadingModel;

    uint AlbedoTexIndex;
    uint NormalTexIndex;
    uint ORMTexIndex;
    uint EmissiveTexIndex;

    uint MaterialFlags;
    uint3 Pad;
};

// ==========================================
// 2. 根签名参数绑定 (Root Parameters)
// ==========================================

cbuffer RootConstants : register(b0, space0)
{
    uint g_InstanceIndex;
};

cbuffer GlobalPassData : register(b1, space0)
{
    float4x4 ViewProjectionMatrix;
    float3   CameraPosition;
    float    Pad1;
    float3   SunDirection;
    float    SunIntensity;
    float4   SunColor;
};

StructuredBuffer<InstanceData>     g_Instances : register(t0, space0);
StructuredBuffer<FPBRMaterialData> g_Materials : register(t1, space0);

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Sampler : register(s0, space0);

// ==========================================
// 3. 管线输入/输出结构
// ==========================================
struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VSOutput
{
    float4 PositionCS    : SV_POSITION;
    float3 WorldPos      : POSITION;    // 🔴 新增：用于计算光照方向
    float3 WorldNormal   : NORMAL;      // 🔴 新增：用于计算法线
    float2 TexCoord      : TEXCOORD;
    nointerpolation uint InstanceIndex : BLENDINDICES0;
};

// ==========================================
// 4. PBR 核心数学库 (Cook-Torrance BRDF)
// ==========================================
static const float PI = 3.14159265359;

// 菲涅尔方程 (Fresnel Schlick)
float3 FresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// 法线分布函数 (GGX)
float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return num / denom;
}

// 几何函数 (Schlick-GGX)
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

// ==========================================
// 5. Vertex Shader
// ==========================================
VSOutput VSMain(VSInput input)
{
    VSOutput output;

    InstanceData instData = g_Instances[g_InstanceIndex];

    float4 worldPos = mul(float4(input.Position, 1.0f), instData.WorldMatrix);
    output.PositionCS = mul(worldPos, ViewProjectionMatrix);

    // 传递给 PS 的世界空间数据
    output.WorldPos = worldPos.xyz;
    // 假设是统一缩放，直接用 float3x3 转换法线
    output.WorldNormal = normalize(mul(input.Normal, (float3x3)instData.WorldMatrix));

    output.TexCoord = input.TexCoord;
    output.InstanceIndex = g_InstanceIndex;

    return output;
}

// ==========================================
// 6. Pixel Shader
// ==========================================
struct PSOutput
{
    float4 ColorTarget : SV_TARGET0;
};

PSOutput PSMain(VSOutput input)
{
    PSOutput output;

    // 1. 抓取数据
    InstanceData instData = g_Instances[input.InstanceIndex];
    FPBRMaterialData matData = g_Materials[instData.MaterialIndex];

    // 2. 采样贴图
    float4 albedoMap = g_Textures[NonUniformResourceIndex(matData.AlbedoTexIndex)].Sample(g_Sampler, input.TexCoord);
    float4 normalMap = g_Textures[NonUniformResourceIndex(matData.NormalTexIndex)].Sample(g_Sampler, input.TexCoord);
    float4 ormMap    = g_Textures[NonUniformResourceIndex(matData.ORMTexIndex)].Sample(g_Sampler, input.TexCoord);

    // 3. glTF 材质解包
    float3 albedo    = albedoMap.rgb * matData.BaseColorFactor.rgb;
    // glTF ORM 规范：R=AO (环境光遮蔽), G=Roughness (粗糙度), B=Metallic (金属度)
    float ao         = ormMap.r;
    float roughness  = ormMap.g * matData.RoughnessFactor;
    float metallic   = ormMap.b * matData.MetallicFactor;

    // 4. 🌟 魔法：利用偏导数计算 TBN 矩阵 (无需 C++ 传切线！)
    float3 Q1  = ddx(input.WorldPos);
    float3 Q2  = ddy(input.WorldPos);
    float2 st1 = ddx(input.TexCoord);
    float2 st2 = ddy(input.TexCoord);

    float3 N   = normalize(input.WorldNormal);
    float3 T   = normalize(Q1 * st2.y - Q2 * st1.y);
    float3 B   = -normalize(cross(N, T)); // 根据具体纹理坐标系可能需要去除负号
    float3x3 TBN = float3x3(T, B, N);

    // 将采样到的法线从 [0, 1] 映射到 [-1, 1]，并转到世界空间
    float3 tangentNormal = normalMap.xyz * 2.0 - 1.0;
    float3 worldNormal = normalize(mul(tangentNormal, TBN));

    // 5. PBR 光照准备
    float3 V = normalize(CameraPosition - input.WorldPos);
    // 注意：假设你的 SunDirection 存的是“光照射向物体的方向”，这里需要取反变成“指向光源的方向”
    float3 L = normalize(-SunDirection);
    float3 H = normalize(V + L);

    // 基础反射率计算 (绝缘体默认 0.04，金属使用 Albedo)
    float3 F0 = float3(0.04, 0.04, 0.04);
    F0 = lerp(F0, albedo, metallic);

    // 6. Cook-Torrance BRDF 计算
    float NDF = DistributionGGX(worldNormal, H, roughness);
    float G   = GeometrySmith(worldNormal, V, L, roughness);
    float3 F  = FresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 numerator    = NDF * G * F;
    float denominator   = 4.0 * max(dot(worldNormal, V), 0.0) * max(dot(worldNormal, L), 0.0) + 0.0001; // 防止除零
    float3 specular     = numerator / denominator;

    // 漫反射与高光能量守恒
    float3 kS = F;
    float3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;	// 纯金属没有漫反射

    // 7. 定向光 (Sun) 最终贡献
    float NdotL = max(dot(worldNormal, L), 0.0);
    // 入射光强度 (Radiance)
    float3 radiance = SunColor.rgb * SunIntensity;
    float3 Lo = (kD * albedo / PI + specular) * radiance * NdotL;

    // 8. 简单的环境光 (Ambient) + AO 遮蔽
    float3 ambient = float3(0.03, 0.03, 0.03) * albedo * ao;
    float3 color = ambient + Lo;

    // 9. HDR 色调映射 (Tone Mapping) - Reinhard 算法
    // 防止高光过曝，将颜色从无限范围压回 [0, 1]
    color = color / (color + 1.0);

    // 10. Gamma 校正
    color = pow(color, float3(1.0/2.2, 1.0/2.2, 1.0/2.2));

    output.ColorTarget = float4(color, 1.0f);
    return output;
}