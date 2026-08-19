struct FDebugConstants
{
    uint DepthSRVIndex;
    uint OutputUAVIndex;
    uint Width;
    uint Height;
};
ConstantBuffer<FDebugConstants> Constants : register(b0, space0);

Texture2D<float> gTextures[] : register(t0, space1);
RWTexture2D<unorm float4> gRWTextures[] : register(u0, space3);

[numthreads(8, 8, 1)]
void CSMain(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    if (DispatchThreadID.x >= Constants.Width || DispatchThreadID.y >= Constants.Height)
        return;

    const float RawDepth = gTextures[Constants.DepthSRVIndex][DispatchThreadID.xy];

    const float Visualized = pow(saturate(RawDepth), 16.0f);

    gRWTextures[Constants.OutputUAVIndex][DispatchThreadID.xy] = float4(Visualized, Visualized, Visualized, 1.0f);
}