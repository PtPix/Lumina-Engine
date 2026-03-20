//Global Per Frame Data
cbuffer ViewUniformData : register(b0)
{
    matrix ViewProj;
    matrix InverseViewProj;
    float4 CameraPosition;
    float2 ScreenResolution;
    float2 _Pad;
};

cbuffer DirectionalLight : register(b1)
{
    float4 LightDirection;
    float4 LightColor;
};

// Per Object Data
cbuffer PrimitiveObjectData : register(b2)
{
    matrix ModelMatrix;
};

// Per Material Data
cbuffer MaterialData : register(b3)
{
    float4 BaseColor;
    float Metallic;
    float Roughness;
    float AO;
    float Padding;
};

// PBR Textures
Texture2D t_Albedo : register(t0);
Texture2D t_Normal : register(t1);
Texture2D t_MetalRough : register(t2);
Texture2D t_Emissive : register(t3);
Texture2D t_AO : register(t4);

// Global Texture
Texture2D t_Skybox : register(t5);

// Static Sampler
SamplerState gSamplerAniso : register(s0);
SamplerState gSamplerPoint : register(s1);