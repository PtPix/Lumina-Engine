#pragma once
#include <cstdint>

#include "RHITypes.h"
#include "Renderer/RHI/common/Containers.h"

namespace LuminaRHI
{
    // Blend State
    enum class EBlendFactor : uint8_t
    {
        Zero = 1,
        One = 2,
        SrcColor = 3,
        InvSrcColor = 4,
        SrcAlpha = 5,
        InvSrcAlpha = 6,
        DstAlpha  = 7,
        InvDstAlpha = 8,
        DstColor = 9,
        InvDstColor = 10,
        SrcAlphaSaturate = 11,
        ConstantColor = 14,
        InvConstantColor = 15,
        Src1Color = 16,
        InvSrc1Color = 17,
        Src1Alpha = 18,
        InvSrc1Alpha = 19
    };

    enum class EBlendOp : uint8_t
    {
        Add = 1,
        Subtract = 2,
        ReverseSubtract = 3,
        Min = 4,
        Max = 5
    };

    enum class EColorMask : uint8_t
    {
        Red = 1,
        Green = 2,
        Blue = 4,
        Alpha = 8,
        All = 0xF
    };

    struct FBlendState
    {
        struct FRenderTarget
        {
            bool        bBlendEnable = false;
            EBlendFactor SrcBlend = EBlendFactor::One;
            EBlendFactor DestBlend = EBlendFactor::Zero;
            EBlendOp     BlendOp = EBlendOp::Add;
            EBlendFactor SrcBlendAlpha = EBlendFactor::One;
            EBlendFactor DestBlendAlpha = EBlendFactor::Zero;
            EBlendOp     BlendOpAlpha = EBlendOp::Add;
            EColorMask   ColorWriteMask = EColorMask::All;

            constexpr FRenderTarget& SetBlendEnable(bool enable) { bBlendEnable = enable; return *this; }
            constexpr FRenderTarget& EnableBlend() { bBlendEnable = true; return *this; }
            constexpr FRenderTarget& DisableBlend() { bBlendEnable = false; return *this; }
            constexpr FRenderTarget& SetSrcBlend(EBlendFactor value) { SrcBlend = value; return *this; }
            constexpr FRenderTarget& SetDestBlend(EBlendFactor value) { DestBlend = value; return *this; }
            constexpr FRenderTarget& SetBlendOp(EBlendOp value) { BlendOp = value; return *this; }
            constexpr FRenderTarget& SetSrcBlendAlpha(EBlendFactor value) { SrcBlendAlpha = value; return *this; }
            constexpr FRenderTarget& SetDestBlendAlpha(EBlendFactor value) { DestBlendAlpha = value; return *this; }
            constexpr FRenderTarget& SetBlendOpAlpha(EBlendOp value) { BlendOpAlpha = value; return *this; }
            constexpr FRenderTarget& SetColorWriteMask(EColorMask value) { ColorWriteMask = value; return *this; }

            constexpr bool operator ==(const FRenderTarget& other) const
            {
                return bBlendEnable == other.bBlendEnable
                    && SrcBlend == other.SrcBlend
                    && DestBlend == other.DestBlend
                    && BlendOp == other.BlendOp
                    && SrcBlendAlpha == other.SrcBlendAlpha
                    && DestBlendAlpha == other.DestBlendAlpha
                    && BlendOpAlpha == other.BlendOpAlpha
                    && ColorWriteMask == other.ColorWriteMask;
            }

            constexpr bool operator !=(const FRenderTarget& other) const
            {
                return !(*this == other);
            }
        };

        FRenderTarget Targets[8];
        bool AlphaToCoverageEnable = false;

        constexpr FBlendState& SetRenderTarget(uint32_t Index, const FRenderTarget& target) { Targets[Index] = target; return *this; }
        constexpr FBlendState& SetAlphaToCoverageEnable(bool Enable) { AlphaToCoverageEnable = Enable; return *this; }
        constexpr FBlendState& EnableAlphaToCoverage() { AlphaToCoverageEnable = true; return *this; }
        constexpr FBlendState& DisableAlphaToCoverage() { AlphaToCoverageEnable = false; return *this; }

        [[nodiscard]] bool UsesConstantColor(uint32_t NumTargets) const;

        constexpr bool operator ==(const FBlendState& Other) const
        {
            if (AlphaToCoverageEnable != Other.AlphaToCoverageEnable)
                return false;

            for (uint32_t i = 0; i < 8; ++i)
            {
                if (Targets[i] != Other.Targets[i])
                    return false;
            }

            return true;
        }

        constexpr bool operator !=(const FBlendState& Other) const
        {
            return !(*this == Other);
        }
    };

    // Raster State
    enum class ERasterFillMode : uint8_t
    {
        Solid,
        Wireframe
    };

    enum class ERasterCullMode : uint8_t
    {
        Back,
        Front,
        None
    };

    struct FRasterState
    {
        ERasterFillMode FillMode = ERasterFillMode::Solid;
        ERasterCullMode CullMode = ERasterCullMode::Back;
        bool bFrontCounterClockwise = false;
        bool bDepthClipEnable = false;
        bool bScissorEnable = false;
        bool bMultisampleEnable = false;
        bool bAntiAliasedLineEnable = false;
        int DepthBias = 0;
        float DepthBiasClamp = 0.0f;

        constexpr FRasterState& SetFillMode(ERasterFillMode Value) { FillMode = Value; return *this; }
        constexpr FRasterState& SetFillSolid() { FillMode = ERasterFillMode::Solid; return *this; }
        constexpr FRasterState& SetFillWireframe() { FillMode = ERasterFillMode::Wireframe; return *this; }
        constexpr FRasterState& SetCullMode(ERasterCullMode Value) { CullMode = Value; return *this; }
        constexpr FRasterState& SetCullBack() { CullMode = ERasterCullMode::Back; return *this; }
        constexpr FRasterState& SetCullFront() { CullMode = ERasterCullMode::Front; return *this; }
        constexpr FRasterState& SetCullNone() { CullMode = ERasterCullMode::None; return *this; }
        constexpr FRasterState& SetFrontCounterClockwise(bool Value) { bFrontCounterClockwise = Value; return *this; }
        constexpr FRasterState& SetDepthClipEnable(bool Value) { bDepthClipEnable = Value; return *this; }
        constexpr FRasterState& SetScissorEnable(bool Value) { bScissorEnable = Value; return *this; }
        constexpr FRasterState& SetMultisampleEnable(bool Value) { bMultisampleEnable = Value; return *this; }
        constexpr FRasterState& SetAntiAliasedLineEnable(bool Value) { bAntiAliasedLineEnable = Value; return *this; }
        constexpr FRasterState& SetDepthBias(float Value) { DepthBias = Value; return *this; }
        constexpr FRasterState& SetDepthBiasClamp(float Value) { DepthBias = Value; return *this; }
    };

    // Depth Stencil State
    enum class EStencilOp : uint8_t
    {
        Keep = 1,
        Zero = 2,
        Replace = 3,
        IncrementAndClamp = 4,
        DecrementAndClamp = 5,
        Invert = 6,
        IncrementAndWrap = 7,
        DecrementAndWrap = 8
    };

    enum class EComparisonFunc : uint8_t
    {
        Never = 1,
        Less = 2,
        Equal = 3,
        LessOrEqual = 4,
        Greater = 5,
        NotEqual = 6,
        GreaterOrEqual = 7,
        Always = 8
    };

    struct FDepthStencilState
    {
        struct FStencilOpDesc
        {
            EStencilOp FailOp = EStencilOp::Keep;
            EStencilOp DepthFailOp = EStencilOp::Keep;
            EStencilOp PassOp = EStencilOp::Keep;
            EComparisonFunc Func = EComparisonFunc::Always;

            constexpr FStencilOpDesc& SetFailOp(EStencilOp Value) { FailOp = Value; return *this; }
            constexpr FStencilOpDesc& SetDepthFailOp(EStencilOp Value) { DepthFailOp = Value; return *this; }
            constexpr FStencilOpDesc& SetPassOp(EStencilOp Value) { PassOp = Value; return *this; }
            constexpr FStencilOpDesc& SetStencilFunc(EComparisonFunc Value) { Func = Value; return *this; }
        };

        bool DepthTestEnable = true;
        bool DepthWriteEnable = true;
        EComparisonFunc DepthFunc = EComparisonFunc::Less;
        bool bStencilEnable = false;
        uint8_t StencilReadMask = 0xFF;
        uint8_t StencilWriteMask = 0xFF;
        uint8_t StencilRefValue = 0;

        constexpr FDepthStencilState& SetDepthTestEnable(bool Value) { DepthTestEnable = Value; return *this; }
        constexpr FDepthStencilState& EnableDepthTest() { DepthTestEnable = true; return *this; }
        constexpr FDepthStencilState& DisableDepthTest() { DepthTestEnable = false; return *this; }
        constexpr FDepthStencilState& SetDepthWriteEnable(bool Value) { DepthWriteEnable = Value; return *this; }
        constexpr FDepthStencilState& EnableDepthWrite() { DepthWriteEnable = true; return *this; }
        constexpr FDepthStencilState& DisableDepthWrite() { DepthWriteEnable = false; return *this; }
        constexpr FDepthStencilState& SetDepthFunc(EComparisonFunc Value) { DepthFunc = Value; return *this; }
        constexpr FDepthStencilState& SetStencilEnable(bool Value) { bStencilEnable = Value; return *this; }
        constexpr FDepthStencilState& EnableStencil() { bStencilEnable = true; return *this; }
        constexpr FDepthStencilState& DisableStencil() { bStencilEnable = false; return *this; }
        constexpr FDepthStencilState& SetStencilReadMask(uint8_t Value) { StencilReadMask = Value; return *this; }
        constexpr FDepthStencilState& SetStencilWriteMask(uint8_t Value) { StencilWriteMask = Value; return *this; }
        constexpr FDepthStencilState& SetStencilRefValue(uint8_t Value) { StencilRefValue = Value; return *this; }
    };

    // Viewport State
    struct FViewportState
    {
        static_vector<FViewport, 16> Viewports;
        static_vector<FRect, 16> ScissorRects;

        FViewportState& AddViewport(const FViewport& Value) { Viewports.push_back(Value); return *this; }
        FViewportState& AddScissorRect(const FRect& Value) { ScissorRects.push_back(Value); return *this; }
    };
}
