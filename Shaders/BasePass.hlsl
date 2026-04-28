// Assets/Shaders/BasePass.hlsl

// 1. 数据结构必须与 C++ 侧的 FObjectData 严格保持 16 字节对齐
struct ObjectData
{
    float4x4 WorldMatrix;
    float4   BaseColor;
    float    MetallicMultiplier;
    float    RoughnessMultiplier;
    uint     AlbedoTexIndex;
    uint     NormalTexIndex;
    // 隐式补齐对齐
};

// 2. 全局参数 (对应 Parameter 2: Root CBV)
cbuffer GlobalPassData : register(b1, space0)
{
    float4x4 ViewProjectionMatrix;
    float3   CameraPosition;
    float    Pad1;
    float3   SunDirection;
    float    SunIntensity;
    float4   SunColor;
};

// 3. Bindless 索引参数 (对应 Parameter 0: Root Constants)
cbuffer RootConstants : register(b0, space0)
{
    uint g_BindlessIndex;
};

// 4. Bindless 资源海 (对应 Parameter 1: 无界 SRV 数组)
StructuredBuffer<ObjectData> g_GlobalObjectData[] : register(t0, space1);

struct VSInput
{
    float3 Position : POSITION;
    float3 Normal   : NORMAL;
    float2 TexCoord : TEXCOORD;
};

struct VSOutput
{
    float4 PositionHCS : SV_POSITION; // 屏幕裁剪空间坐标
    float3 NormalW     : NORMAL;      // 世界空间法线
    float2 TexCoord    : TEXCOORD;
    // 使用 flat 传递索引给 PS，避免插值开销
    nointerpolation uint BindlessIndex : BLENDINDICES0;
};

// ==========================================
// Vertex Shader
// ==========================================
VSOutput VSMain(VSInput input)
{
    VSOutput output;

    // 【核心抓取】：通过传进来的门牌号，拿到该物体特有的数据
    ObjectData myData = g_GlobalObjectData[NonUniformResourceIndex(g_BindlessIndex)][0];

    // 矩阵计算：Local -> World -> Clip
    float4 worldPos = mul(float4(input.Position, 1.0f), myData.WorldMatrix);
    output.PositionHCS = mul(worldPos, ViewProjectionMatrix);

    output.NormalW = mul(input.Normal, (float3x3)myData.WorldMatrix);
    output.TexCoord = input.TexCoord;
    output.BindlessIndex = g_BindlessIndex; // 传给 PS

    return output;
}

// ==========================================
// Pixel Shader (BasePass - 输出到 G-Buffer/RT)
// ==========================================
struct PSOutput
{
    float4 ColorTarget : SV_TARGET0;
    // 未来在这里增加 SV_TARGET1 (Normal), SV_TARGET2 (M/R) 等实现完整的 G-Buffer
};

PSOutput PSMain(VSOutput input)
{
    PSOutput output;

    // PS 中再次抓取当前物体数据
    ObjectData myData = g_GlobalObjectData[NonUniformResourceIndex(input.BindlessIndex)][0];

    // BasePass 阶段主要负责向 RT 写入基础属性。
    // 当前测试阶段，我们直接将 Buffer 中携带的 BaseColor 写入 RT0。
    output.ColorTarget = myData.BaseColor;

    return output;
}