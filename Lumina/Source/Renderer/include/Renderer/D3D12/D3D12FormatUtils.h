#pragma once

#include <dxgiformat.h>
#include <cstdint>
#include <algorithm>

namespace D3D12FormatUtils
{
    inline bool IsDepthFormat(DXGI_FORMAT Format)
    {
        switch (Format)
        {
            case DXGI_FORMAT_D16_UNORM:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_R32_TYPELESS:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_R16_TYPELESS:
            case DXGI_FORMAT_R32G8X24_TYPELESS:
                return true;
            default:
                return false;
        }
    }

    inline bool HasStencil(DXGI_FORMAT Format)
    {
        switch (Format)
        {
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_R24G8_TYPELESS:
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_R32G8X24_TYPELESS:
                return true;
            default:
                return false;
        }
    }

    inline bool IsSRGBFormat(DXGI_FORMAT Format)
    {
        switch (Format)
        {
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
            case DXGI_FORMAT_BC1_UNORM_SRGB:
            case DXGI_FORMAT_BC2_UNORM_SRGB:
            case DXGI_FORMAT_BC3_UNORM_SRGB:
            case DXGI_FORMAT_BC7_UNORM_SRGB:
                return true;
            default:
                return false;
        }
    }

    // Depth Resources
    inline DXGI_FORMAT GetResourceFormat(DXGI_FORMAT Format)
    {
        switch (Format)
        {
            case DXGI_FORMAT_D16_UNORM:           return DXGI_FORMAT_R16_TYPELESS;
            case DXGI_FORMAT_D24_UNORM_S8_UINT:   return DXGI_FORMAT_R24G8_TYPELESS;
            case DXGI_FORMAT_D32_FLOAT:           return DXGI_FORMAT_R32_TYPELESS;
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:return DXGI_FORMAT_R32G8X24_TYPELESS;
            default:                              return Format;
        }
    }

    inline DXGI_FORMAT GetDSVFormat(DXGI_FORMAT Format)
    {
        switch (Format)
        {
            case DXGI_FORMAT_R16_TYPELESS:        return DXGI_FORMAT_D16_UNORM;
            case DXGI_FORMAT_R24G8_TYPELESS:      return DXGI_FORMAT_D24_UNORM_S8_UINT;
            case DXGI_FORMAT_R32_TYPELESS:        return DXGI_FORMAT_D32_FLOAT;
            case DXGI_FORMAT_R32G8X24_TYPELESS:   return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
            default:                              return Format;
        }
    }

    // Depth Resource Samplable Type
    inline DXGI_FORMAT GetSRVFormat(DXGI_FORMAT Format)
    {
        switch (Format)
        {
            case DXGI_FORMAT_D16_UNORM:
            case DXGI_FORMAT_R16_TYPELESS:        return DXGI_FORMAT_R16_UNORM;
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_R24G8_TYPELESS:      return DXGI_FORMAT_R24_UNORM_X8_TYPELESS;
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_R32_TYPELESS:        return DXGI_FORMAT_R32_FLOAT;
            case DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
            case DXGI_FORMAT_R32G8X24_TYPELESS:   return DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS;
            default:                              return Format;
        }
    }

    inline DXGI_FORMAT GetRTVFormat(DXGI_FORMAT Format)
    {
        return GetSRVFormat(Format);
    }

    // UAV
    inline DXGI_FORMAT GetUAVFormat(DXGI_FORMAT Format)
    {
        switch (Format)
        {
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM;
            case DXGI_FORMAT_B8G8R8A8_UNORM_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM;
            default:                              return Format;
        }
    }

    inline bool IsBlockCompressed(DXGI_FORMAT Format)
    {
        return (Format >= DXGI_FORMAT_BC1_TYPELESS && Format <= DXGI_FORMAT_BC5_SNORM)
            || (Format >= DXGI_FORMAT_BC6H_TYPELESS && Format <= DXGI_FORMAT_BC7_UNORM_SRGB);
    }

    inline uint32_t GetBytesPerPixel(DXGI_FORMAT Format)
    {
        switch (Format)
        {
            case DXGI_FORMAT_R32G32B32A32_FLOAT:
            case DXGI_FORMAT_R32G32B32A32_UINT:     return 16;
            case DXGI_FORMAT_R32G32B32_FLOAT:       return 12;
            case DXGI_FORMAT_R16G16B16A16_FLOAT:
            case DXGI_FORMAT_R16G16B16A16_UNORM:
            case DXGI_FORMAT_R32G32_FLOAT:          return 8;
            case DXGI_FORMAT_R8G8B8A8_UNORM:
            case DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
            case DXGI_FORMAT_B8G8R8A8_UNORM:
            case DXGI_FORMAT_R10G10B10A2_UNORM:
            case DXGI_FORMAT_R11G11B10_FLOAT:
            case DXGI_FORMAT_R16G16_FLOAT:
            case DXGI_FORMAT_R32_FLOAT:
            case DXGI_FORMAT_D32_FLOAT:
            case DXGI_FORMAT_R32_TYPELESS:
            case DXGI_FORMAT_D24_UNORM_S8_UINT:
            case DXGI_FORMAT_R24G8_TYPELESS:        return 4;
            case DXGI_FORMAT_R16_FLOAT:
            case DXGI_FORMAT_R16_UNORM:
            case DXGI_FORMAT_D16_UNORM:
            case DXGI_FORMAT_R8G8_UNORM:            return 2;
            case DXGI_FORMAT_R8_UNORM:              return 1;
            default:                                return 4;
        }
    }

    inline uint32_t CalculateNumMips(uint32_t Width, uint32_t Height, uint32_t Depth = 1)
    {
        uint32_t Size = (std::max)(Width, (std::max)(Height, Depth));
        uint32_t Mips = 1;
        while (Size > 1)
        {
            Size >>= 1;
            ++Mips;
        }
        return Mips;
    }
}