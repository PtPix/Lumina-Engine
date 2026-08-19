#include "Renderer/RenderPass/DebugVisualizePass.h"

#include "Renderer/Renderer.h"
#include "Renderer/Pipeline/PipelineStateCache.h"
#include "Renderer/D3D12Core/D3D12Backend.h"
#include "Renderer/D3D12Core/Core/CommandContext.h"
#include "Renderer/D3D12Core/Core/Device.h"
#include "Renderer/D3D12Core/Descriptors/BindlessDescriptorHeap.h"
#include "Renderer/D3D12Core/Pipeline/RootSignature.h"
#include "Renderer/D3D12Core/Resource/Texture.h"

namespace
{
      // A1 阶段的临时局部 compute root signature。
      // A2 会用全局统一的 compute root signature 替换掉它。
      enum class EDebugRootParam : UINT
      {
          Constants     = 0,   // b0 space0 : 4 个 uint
          BindlessTable = 1,   // t0 space1 (SRV) + u0 space3 (UAV)，共享同一 heap
      };

      FRootSignature gDebugRootSignature;
      bool           gbRootSignatureBuilt = false;

      ID3D12RootSignature* GetOrCreateDebugRootSignature()
      {
          if (gbRootSignatureBuilt) return gDebugRootSignature.Get();

          FRootSignatureBuilder Builder;
          Builder.AddRootConstants(0, 0, 4);   // EDebugRootParam::Constants

          std::vector<D3D12_DESCRIPTOR_RANGE1> Ranges;

          D3D12_DESCRIPTOR_RANGE1 SrvRange = {};
          SrvRange.RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
          SrvRange.NumDescriptors     = UINT_MAX;
          SrvRange.BaseShaderRegister = 0;
          SrvRange.RegisterSpace      = 1;
          SrvRange.Flags              = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE |
                                        D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
          SrvRange.OffsetInDescriptorsFromTableStart = 0;
          Ranges.push_back(SrvRange);

          D3D12_DESCRIPTOR_RANGE1 UavRange = {};
          UavRange.RangeType          = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
          UavRange.NumDescriptors     = UINT_MAX;
          UavRange.BaseShaderRegister = 0;
          UavRange.RegisterSpace      = 3;
          UavRange.Flags              = D3D12_DESCRIPTOR_RANGE_FLAG_DESCRIPTORS_VOLATILE |
                                        D3D12_DESCRIPTOR_RANGE_FLAG_DATA_VOLATILE;
          UavRange.OffsetInDescriptorsFromTableStart = 0;
          Ranges.push_back(UavRange);

          Builder.AddDescriptorTable(Ranges, D3D12_SHADER_VISIBILITY_ALL);

          gbRootSignatureBuilt = Builder.Build(
              Renderer::GetD3D12Backend()->GetDevice()->GetDevice(), gDebugRootSignature);

          return gbRootSignatureBuilt ? gDebugRootSignature.Get() : nullptr;
      }
}

void AddDebugVisualizeDepthPass(FRenderGraph& Graph,
                                  const FDebugVisualizeInputs& Inputs,
                                  FDebugVisualizeOutputs& Outputs)
{
      if (!Inputs.SceneDepth.IsValid()) return;

      const uint32_t Width  = Inputs.Width  ? Inputs.Width  : Renderer::GetD3D12Backend()->GetWidth();
      const uint32_t Height = Inputs.Height ? Inputs.Height : Renderer::GetD3D12Backend()->GetHeight();

      // 输出纹理：既要能被 compute 写(UAV)，也要能被后续 pass / ImGui 读(SRV)
      FRGTextureDesc OutDesc = {};
      OutDesc.Width  = Width;
      OutDesc.Height = Height;
      OutDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
      OutDesc.Usage  = ERGTextureUsage::UnorderedAccess | ERGTextureUsage::ShaderResource;

      Outputs.VisualizedDepth = Graph.CreateTexture("Debug.VisualizedDepth", OutDesc);

      const FRGTextureHandle DepthHandle  = Inputs.SceneDepth;
      const FRGTextureHandle OutputHandle = Outputs.VisualizedDepth;

      Graph.AddPass("DebugVisualizeDepth")
          .ReadTexture(DepthHandle)
          .ReadWriteUAV(OutputHandle)
          .Execute([DepthHandle, OutputHandle, Width, Height](FRenderGraphContext& Context)
          {
              FCommandContext* Cmd = Context.GetCommandContext();
              LUMINA_GPU_EVENT(Cmd, "DebugVisualizeDepth");

              ID3D12RootSignature* pRootSignature = GetOrCreateDebugRootSignature();
              if (!pRootSignature) return;

              FComputePSODesc PSODesc;
              PSODesc.ShaderPath    = L"Shaders/DebugVisualize.hlsl";
              PSODesc.CSEntry       = "CSMain";
              PSODesc.ShaderModel   = EShaderModel::SM6_0;
              PSODesc.RootSignature = pRootSignature;

              FPipelineState* PSO = FPipelineStateCache::GetOrCreate(PSODesc);
              if (!PSO) return;

              const uint32_t DepthSRVIndex  = Context.GetSRVIndex(DepthHandle);
              const uint32_t OutputUAVIndex = Context.GetUAVIndex(OutputHandle, 0);

              if (DepthSRVIndex  == FTexture::InvalidBindlessIndex) return;
              if (OutputUAVIndex == FTexture::InvalidBindlessIndex) return;

              FBindlessDescriptorHeap* pHeap =
                  Renderer::GetD3D12Backend()->GetBindlessDescriptorHeap();

              ID3D12DescriptorHeap* ppHeaps[] = { pHeap->GetDescriptorHeap() };
              Cmd->SetDescriptorHeaps(1, ppHeaps);

              Cmd->SetComputeRootSignature(pRootSignature);
              Cmd->SetPipelineState(PSO->Get());

              const uint32_t Constants[4] = { DepthSRVIndex, OutputUAVIndex, Width, Height };
              Cmd->SetComputeRoot32BitConstants(
                  static_cast<UINT>(EDebugRootParam::Constants), 4, Constants, 0);

              Cmd->SetComputeRootDescriptorTable(
                  static_cast<UINT>(EDebugRootParam::BindlessTable), pHeap->GetGpuHandle(0));

              Cmd->Dispatch((Width + 7) / 8, (Height + 7) / 8, 1);

              // 后续 pass 要读这张图，必须插 UAV barrier
              Cmd->InsertUAVBarrier(Context.GetResource(OutputHandle));
          });
}