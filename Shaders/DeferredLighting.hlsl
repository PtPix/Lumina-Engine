#include "Common.hlsli"

// ==========================================
// 1. 无缓冲全屏三角形 (Bufferless Full-Screen Triangle)
// ==========================================
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 UV : TEXCOORD;
};

// 💡 黑魔法：不读取任何 Vertex Buffer，直接用 Vertex ID 凭空算出全屏三角形！
// 该三角形大到足以完全覆盖屏幕 [-1, 1] 的裁剪空间
PSInput VSMain(uint VertexID : SV_VertexID)
{
    PSInput Result;
    // 根据 ID (0, 1, 2) 生成 UV: (0,0), (2,0), (0,2)
    Result.UV = float2((VertexID << 1) & 2, VertexID & 2);
    // 将 UV 映射到 NDC 裁剪坐标 (X: -1~3, Y: 1~-3)
    Result.Position = float4(Result.UV * float2(2.0f, -2.0f) + float2(-1.0f, 1.0f), 0.0f, 1.0f);
    return Result;
}

// ==========================================
// 2. G-Buffer 纹理声明
// ==========================================
Texture2D t_SceneColor : register(t0); // RT0: Emissive / Skybox
Texture2D t_GBufferA   : register(t1); // RT1: Normal
Texture2D t_GBufferB   : register(t2); // RT2: Metallic, Specular, Roughness
Texture2D t_GBufferC   : register(t3); // RT3: BaseColor, AO
Texture2D t_Depth      : register(t4); // DSV: Depth

// ==========================================
// 3. 常量缓冲与 PBR 辅助函数 (部分复用)
// ==========================================
#define PI 3.14159265359f

float DistributionGGX(float3 N, float3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;
    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
    return nom / max(denom, 0.0000001);
}

float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;
    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;
    return nom / denom;
}

float GeometrySmith(float3 N, float3 V, float3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

float3 fresnelSchlick(float cosTheta, float3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// ==========================================
// 4. 光照计算核心 (Pixel Shader)
// ==========================================
float4 PSMain(PSInput Input) : SV_TARGET
{
    // 💡 极其关键：读取 G-Buffer 必须使用 Point Clamp 采样器 (gSamplerPoint 通常注册在 s1)
    // 绝对不能用线性插值，否则物体的边缘属性会互相污染！
    int3 texCoord = int3(Input.Position.xy, 0);

    float4 SceneColor = t_SceneColor.Load(texCoord);
    float depth       = t_Depth.Load(texCoord).r;

    // 剔除天空盒背景：如果深度为 1.0 (最远端)，说明这里没有模型，直接输出天空盒颜色
    if (depth >= 1.0f)
    {
        return SceneColor;
    }

    // 1. 从 G-Buffer 解码材质属性
    float4 GBufferA = t_GBufferA.Load(texCoord);
    float4 GBufferB = t_GBufferB.Load(texCoord);
    float4 GBufferC = t_GBufferC.Load(texCoord);

    // 法线映射回 [-1, 1] 范围
    float3 WorldNormal = GBufferA.rgb * 2.0f - 1.0f;

    float Metallic  = GBufferB.r;
    float Specular  = GBufferB.g;
    float Roughness = GBufferB.b;
    float ShadingID = GBufferB.a; // 目前全是 Default Lit，预留给后续区分 Unlit 等模型

    float3 BaseColor = GBufferC.rgb;
    float AO         = GBufferC.a;

    // 2. 🌟 究极黑魔法：从深度 (Depth) 反推世界空间坐标 (World Position)
    // 坐标映射：UV (0~1) -> NDC (-1~1，Y轴需翻转)
    float2 clipSpaceUV = Input.UV * 2.0f - 1.0f;
    clipSpaceUV.y = -clipSpaceUV.y;

    // 组装裁剪空间坐标：(x, y, depth, 1.0)
    float4 clipSpacePos = float4(clipSpaceUV, depth, 1.0f);

    // 乘以逆视图投影矩阵，变回世界空间
    float4 worldSpacePos = mul(clipSpacePos, InverseViewProj);
    // 透视除法
    float3 WorldPos = worldSpacePos.xyz / worldSpacePos.w;

    // 3. 执行标准 PBR 光照公式 (与之前完全一致，但现在是在屏幕空间运行！)
    float3 N = normalize(WorldNormal);
    float3 V = normalize(CameraPosition.xyz - WorldPos);
    float3 L = normalize(-LightDirection.xyz);
    float3 H = normalize(V + L);

    // UE 规范的 F0 计算
    float3 F0 = 0.16f * Specular * Specular;
    F0 = lerp(F0, BaseColor, Metallic);

    // 辐射率 (单光源)
    float3 Radiance = LightColor.rgb;

    // BRDF
    float NDF = DistributionGGX(N, H, Roughness);
    float G   = GeometrySmith(N, V, L, Roughness);
    float3 F  = fresnelSchlick(max(dot(H, V), 0.0), F0);

    float3 numerator    = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    float3 specularReflection = numerator / denominator;

    float3 kS = F;
    float3 kD = float3(1.0, 1.0, 1.0) - kS;
    kD *= 1.0 - Metallic;

    float NdotL = max(dot(N, L), 0.0);
    float3 Lo = (kD * BaseColor / PI + specularReflection) * Radiance * NdotL;

    float3 R = reflect(-V, N);

    // 2. 将 3D 反射向量映射到 2D 的全景图 UV (Equirectangular Mapping)
    float2 envUV = float2(atan2(R.z, -R.x), asin(clamp(R.y, -0.9999f, 0.9999f)));
    envUV *= float2(0.1591f, 0.3183f);
    envUV += 0.5f;

    // 3. 采样天空盒
    // 💡 技巧：在全屏 Quad 中采样最好使用 SampleLevel 强制指定 mip 为 0，避免屏幕边缘出现梯度错误警告
    float3 envColor = t_Skybox.SampleLevel(gSamplerAniso, envUV, 0).rgb;

    // 4. 计算环境光菲涅尔 (视角与法线的夹角)
    float3 kS_IBL = fresnelSchlick(max(dot(N, V), 0.0), F0);

    // 5. 结合粗糙度压暗反射 (因为没有做粗糙度预滤波贴图，这里用一种经验公式近似模糊效果)
    float3 specularIBL = envColor * kS_IBL * (1.0 - Roughness);

    // 6. 漫反射环境光 (可以用低级别 Mip 的环境贴图，这里暂用定值替代)
    float3 Ambient = float3(0.05, 0.05, 0.05) * BaseColor * AO;

    // 将天空盒反射加入环境光
    Ambient += specularIBL;
    // 4. 组合最终颜色：环境光 + 漫反射/高光 + 自发光(来自于 SceneColor)
    // float3 Ambient = float3(0.05, 0.05, 0.05) * BaseColor * AO;
    float3 Color = Ambient + Lo + SceneColor.rgb;

    // 5. 简单的 ACES Tonemapping 替代 (防止高光爆掉)
    Color = Color / (Color + float3(1.0, 1.0, 1.0));
    // 伽马校正
    Color = pow(Color, float3(1.0 / 2.2, 1.0 / 2.2, 1.0 / 2.2));

    return float4(Color, 1.0);
}