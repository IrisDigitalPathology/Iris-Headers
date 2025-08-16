/**
 * @file IrisSIMD.cpp
 * @author ryanlandvater (ryanlandvater [at] gmail [dot] com)
 * @brief Iris SIMD operations implementation file using
 * google highway library.
 * @version 0.1
 * @date 2023-10-05
 *
 * @copyright Copyright (c) 2023-2025
 *
 */
#include <stddef.h>
#include <assert.h>

#include "hwy/highway.h"
#include "IrisCore.hpp"
#include "IrisBuffer.hpp"
struct uint8x3_t {
    uint8_t v0;
    uint8_t v1;
    uint8_t v2;
    uint8x3_t (uint8_t* p) {
        *this = *reinterpret_cast<uint8x3_t*>(p);
    }
};
struct uint8x4_t {
    uint8_t v0;
    uint8_t v1;
    uint8_t v2;
    uint8_t a;
    uint8x4_t (uint8_t* p) {
        *this = *reinterpret_cast<uint8x4_t*>(p);
    }
};
HWY_BEFORE_NAMESPACE();
namespace Iris {
namespace SIMD {
using namespace hwy;
using namespace hwy::HWY_NAMESPACE;
namespace HWY_NAMESPACE {

// Helper functions to resolve overload ambiguity
template<typename D>
HWY_INLINE void LoadInterleaved3Helper(D d, const uint8_t* HWY_RESTRICT src,
                                     Vec<D>& v0, Vec<D>& v1, Vec<D>& v2) {
    LoadInterleaved3(d, src, v0, v1, v2);
}

template<typename D>
HWY_INLINE void LoadInterleaved4Helper(D d, const uint8_t* HWY_RESTRICT src,
                                     Vec<D>& v0, Vec<D>& v1, Vec<D>& v2, Vec<D>& v3) {
    LoadInterleaved4(d, src, v0, v1, v2, v3);
}

template<typename D>
HWY_INLINE void StoreInterleaved3Helper(D d, const Vec<D>& v0, const Vec<D>& v1, const Vec<D>& v2,
                                      uint8_t* HWY_RESTRICT dst) {
    StoreInterleaved3(v0, v1, v2, d, dst);
}

template<typename D>
HWY_INLINE void StoreInterleaved4Helper(D d, const Vec<D>& v0, const Vec<D>& v1,
                                      const Vec<D>& v2, const Vec<D>& v3,
                                      uint8_t* HWY_RESTRICT dst) {
    StoreInterleaved4(v0, v1, v2, v3, d, dst);
}

HWY_API void EXPAND_TILE_ADD_ALPHA_8bit (const uint8_t* src, uint8_t* dst)
{
    // This is done BACKWARDS so that it is safe
    // when src and dst are the same pointers (IE can be done from same buffer)
    const ScalableTag<uint8_t> d8;
    const auto N = static_cast<int32_t>(Lanes(d8));
    const auto a = Set(d8, 0xFF);
    Vec<ScalableTag<uint8_t>> v0,v1,v2;
    int32_t i = TILE_PIX_AREA-(N+1);
    for (; i - N >= 0; i -= N) {
        LoadInterleaved3Helper(d8, src + i * 3, v0, v1, v2);
        StoreInterleaved4Helper(d8, v0, v1, v2, a, dst + i * 4);
    }
    for (; i >= 0; --i) {
        memcpy(dst + i * 4, src + i * 3, 3);
        dst[i * 4 + 3] = 0xFF;
    }
}
HWY_API void SHRINK_TILE_RM_ALPHA_8bit (const uint8_t* src, uint8_t* dst)
{
    // This is done FORWARDS so that it is safe
    // when src and dst are the same pointers (IE can be done from same buffer)
    const ScalableTag<uint8_t> d8;
    const auto N = Lanes(d8);
    uint32_t   i = 0;
    Vec<ScalableTag<uint8_t>> v0,v1,v2,a;
    for (; i + N < TILE_PIX_AREA; i+=N) {
        LoadInterleaved4Helper(d8, src + i * 4, v0, v1, v2, a);
        StoreInterleaved3Helper(d8, v0, v1, v2, dst + i * 3);
    } for (; i < TILE_PIX_AREA; ++i)
        memcpy(dst + i * 3, src + i * 4, 3);
    
}
HWY_API void SWAP_TILE_3_CHANNELS_0_2_8bit (uint8_t* src)
{
    const ScalableTag<uint8_t> d8;
    const auto N = Lanes(d8);
    uint32_t   i = 0;
    Vec<ScalableTag<uint8_t>> v0,v1,v2;
    for (; i + N < TILE_PIX_AREA * 3; i += N * 3) {
        LoadInterleaved3Helper(d8, src + i, v0, v1, v2);
        StoreInterleaved3Helper(d8, v2, v1, v0, src + i);
    } for (; i < TILE_PIX_AREA * 3; i += 3) {
        uint8x3_t _ (src + i);
        src[i]      = _.v2;
        src[i + 2]  = _.v0;
    }
}
HWY_API void SWAP_TILE_4_CHANNELS_0_2_8bit (uint8_t* src)
{
    const ScalableTag<uint8_t> d8;
    const auto N = Lanes(d8);
    uint32_t   i = 0;
    Vec<ScalableTag<uint8_t>> v0,v1,v2, a;
    for (; i + N < TILE_PIX_AREA * 4; i += N * 4) {
        LoadInterleaved4Helper(d8, src + i, v0, v1, v2, a);
        StoreInterleaved4Helper(d8, v2, v1, v0, a, src + i);
    } for (; i < TILE_PIX_AREA * 4; i += 4) {
        uint8x3_t _ (src + i);
        src[i]      = _.v2;
        src[i + 2]  = _.v0;
    }
}

template<uint8_t CH>
HWY_API void DOWNSAMPLE_INTO_TILE_2X_AVG(const uint8_t* HWY_RESTRICT src,
                                       uint8_t* HWY_RESTRICT dst,
                                       const uint16_t s_y,
                                       const uint16_t s_x) {
    static_assert(CH == 3 || CH == 4, "Only 3 (RGB) or 4 (RGBA) channels supported");
    const uint8_t o_y = s_y << 7;   // sub-x region [0,1] * 128 pixels
    const uint8_t o_x = s_x << 7;   // sub-y region [0,1] * 128 pixels
    constexpr auto stride = TILE_PIX_LENGTH * CH;
    
    const ScalableTag<uint16_t> d16;
    const FixedTag<uint8_t, d16.MaxLanes()> d8;
    const size_t N = Lanes(d16);
    
    const auto twos = Set(d16, 2);
    
    // Aligned buffers for both input and output
    alignas(32) uint8_t aligned_src[2 * N];
    alignas(32) uint8_t aligned_dst[N];
    
    for (auto y = 0; y < 128; ++y) {
        const auto row0 = src + (2 * y) * stride;
        const auto row1 = src + (2 * y + 1) * stride;
        auto orow = dst + (y + o_y) * stride + o_x * CH;
        
        size_t x = 0;
        for (; x * N < 128 * CH - N; x += N) {
            // Copy first row data to aligned buffer
            memcpy(aligned_src, row0 + 2 * x, N);
            memcpy(aligned_src + N, row0 + 2 * x + CH, N);
            
            // Load and sum first row
            auto sum = PromoteTo(d16, Load(d8, aligned_src));
            sum += PromoteTo(d16, Load(d8, aligned_src + N));
            
            // Copy second row data
            memcpy(aligned_src, row1 + 2 * x, N);
            memcpy(aligned_src + N, row1 + 2 * x + CH, N);
            
            // Add second row
            sum += PromoteTo(d16, Load(d8, aligned_src));
            sum += PromoteTo(d16, Load(d8, aligned_src + N));
            
            // Average and store
            Store(DemoteTo(d8, (sum + twos) >> twos), d8, aligned_dst);
            memcpy(orow + x, aligned_dst, N);
        }
        
        // Scalar cleanup
        for (; x < 128 * CH; x += CH) {
            for (int c = 0; c < CH; ++c) {
                const uint16_t sum =
                    row0[2 * x + c] + row0[2 * x + CH + c] +
                    row1[2 * x + c] + row1[2 * x + CH + c];
                orow[x + c] = static_cast<uint8_t>((sum + 2) >> 2);
            }
        }
    }
}
HWY_API void DOWNSAMPLE_INTO_TILE_2X_AVG_3(const uint8_t* HWY_RESTRICT src,
                                                 uint8_t* HWY_RESTRICT dst,
                                                 const uint16_t s_y,
                                                 const uint16_t s_x) {
    DOWNSAMPLE_INTO_TILE_2X_AVG<3>(src, dst, s_y, s_x);
}

HWY_API void DOWNSAMPLE_INTO_TILE_2X_AVG_4(const uint8_t* HWY_RESTRICT src,
                                                 uint8_t* HWY_RESTRICT dst,
                                                 const uint16_t s_y,
                                                 const uint16_t s_x) {
    DOWNSAMPLE_INTO_TILE_2X_AVG<4>(src, dst, s_y, s_x);
}

template<uint8_t CH>
inline void DOWNSAMPLE_INTO_TILE_4X_AVG(const uint8_t* HWY_RESTRICT src,
                                       uint8_t* HWY_RESTRICT dst,
                                       const uint16_t s_y,
                                       const uint16_t s_x) {
    static_assert(CH == 3 || CH == 4, "Only 3 (RGB) or 4 (RGBA) channels supported");
    
    const uint8_t o_y = s_y << 6;   // sub-x region [0,3] * 64 pixels
    const uint8_t o_x = s_x << 6;   // sub-y region [0,3] * 64 pixels
    constexpr auto stride = TILE_PIX_LENGTH * CH;
    
    const ScalableTag<uint16_t> d16;
    const FixedTag<uint8_t, d16.MaxLanes()> d8;
    const size_t N = Lanes(d16);
    
    const auto shift_amount = Set(d16, 4);  // Divide by 16
    const auto round = Set(d16, 8);         // Round by adding 8
    
    // Ensure alignment for SIMD loads/stores
    alignas(32) uint8_t aligned_src[4 * N];
    alignas(32) uint8_t aligned_dst[N];
    
    for (auto y = 0; y < 64; ++y) {
        const auto row0 = src + (4 * y) * stride;
        const auto row1 = src + (4 * y + 1) * stride;
        const auto row2 = src + (4 * y + 2) * stride;
        const auto row3 = src + (4 * y + 3) * stride;
        auto orow = dst + (y + o_y) * stride + o_x * CH;
        
        size_t x = 0;
        for (; x * N < 64 * CH - N; x += N) {
            // Copy source data to aligned buffer
            memcpy(aligned_src, row0 + 4 * x, N);
            memcpy(aligned_src + N, row0 + 4 * x + CH, N);
            memcpy(aligned_src + 2 * N, row0 + 4 * x + 2 * CH, N);
            memcpy(aligned_src + 3 * N, row0 + 4 * x + 3 * CH, N);
            
            // Load all 16 pixels (4x4 grid)
            auto sum = PromoteTo(d16, Load(d8, aligned_src + 4 * x));
            sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + CH));
            sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + 2 * CH));
            sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + 3 * CH));
            
            // Repeat for other rows...
            memcpy(aligned_src, row1 + 4 * x, N);
            memcpy(aligned_src + N, row1 + 4 * x + CH, N);
            memcpy(aligned_src + 2 * N, row1 + 4 * x + 2 * CH, N);
            memcpy(aligned_src + 3 * N, row1 + 4 * x + 3 * CH, N);
            
            auto row1_sum = PromoteTo(d16, Load(d8, aligned_src + 4 * x));
            row1_sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + CH));
            row1_sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + 2 * CH));
            row1_sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + 3 * CH));
            sum += row1_sum;
            
            memcpy(aligned_src, row2 + 4 * x, N);
            memcpy(aligned_src + N, row2 + 4 * x + CH, N);
            memcpy(aligned_src + 2 * N, row2 + 4 * x + 2 * CH, N);
            memcpy(aligned_src + 3 * N, row2 + 4 * x + 3 * CH, N);
            
            auto row2_sum = PromoteTo(d16, Load(d8, aligned_src + 4 * x));
            row2_sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + CH));
            row2_sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + 2 * CH));
            row2_sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + 3 * CH));
            sum += row2_sum;
            
            memcpy(aligned_src, row3 + 4 * x, N);
            memcpy(aligned_src + N, row3 + 4 * x + CH, N);
            memcpy(aligned_src + 2 * N, row3 + 4 * x + 2 * CH, N);
            memcpy(aligned_src + 3 * N, row3 + 4 * x + 3 * CH, N);
            memcpy(aligned_src + 3 * N, row3 + 4 * x + 3 * CH, N);
            
            auto row3_sum = PromoteTo(d16, Load(d8, aligned_src + 4 * x));
            row3_sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + CH));
            row3_sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + 2 * CH));
            row3_sum += PromoteTo(d16, Load(d8, aligned_src + 4 * x + 3 * CH));
            sum += row3_sum;
            
            // Round and shift
            Store(DemoteTo(d8, (sum + round) >> shift_amount), d8, aligned_dst);
            memcpy(orow + x, aligned_dst, N);
        }
        
        // Scalar cleanup
        for (; x < 64 * CH; x += CH) {
            for (int c = 0; c < CH; ++c) {
                const uint16_t sum =
                    row0[4 * x + c] + row0[4 * x + CH + c] + row0[4 * x + 2*CH + c] + row0[4 * x + 3*CH + c] +
                    row1[4 * x + c] + row1[4 * x + CH + c] + row1[4 * x + 2*CH + c] + row1[4 * x + 3*CH + c] +
                    row2[4 * x + c] + row2[4 * x + CH + c] + row2[4 * x + 2*CH + c] + row2[4 * x + 3*CH + c] +
                    row3[4 * x + c] + row3[4 * x + CH + c] + row3[4 * x + 2*CH + c] + row3[4 * x + 3*CH + c];
                orow[x + c] = static_cast<uint8_t>((sum + 8) >> 4);
            }
        }
    }
}
// Aliases for dynamic dispatch
HWY_API void DOWNSAMPLE_INTO_TILE_4X_AVG_3
 (const uint8_t* HWY_RESTRICT src, uint8_t* HWY_RESTRICT dst, const uint16_t s_y,const uint16_t s_x) {
    DOWNSAMPLE_INTO_TILE_4X_AVG<3>(src, dst, s_y, s_x);
}

HWY_API void DOWNSAMPLE_INTO_TILE_4X_AVG_4
 (const uint8_t* HWY_RESTRICT src, uint8_t* HWY_RESTRICT dst, const uint16_t s_y, const uint16_t s_x) {
    DOWNSAMPLE_INTO_TILE_4X_AVG<4>(src, dst, s_y, s_x);
}
} // END HWY_NAMESPACE
 HWY_AFTER_NAMESPACE();

#if HWY_ONCE
Buffer Convert_tile_format (const Buffer &src, Format s_fmt, Format d_fmt, const Buffer& __dst)
{
    // Destination buffer (assign it to the optional)
    Buffer dst              = __dst;
    
    // No need to convert...
    if (s_fmt == d_fmt) {
        if (!dst || dst->capacity() < src->size())
            dst = src;
        else memcpy(dst->data(), src->data(), src->size());
        dst->set_size(src->size());
        return dst;
    }
    
    // Bitmask the tasks that will be needed
    // ..this will grow. Make sure to account below...
    enum ConversionBits {
        TASK_EXPAND_ALPHA   = 0x01,
        TASK_STRIP_ALPHA    = 0x02,
        TASK_SWAP_0_2       = 0x10,
    };
    // Bitmask
    uint32_t tasks          = 0;
    // Source and dst bits per pixel
    uint8_t s_bpp = 0, d_bpp = 0;
    
    // Tile Sizing
    switch (s_fmt) {
        case FORMAT_UNDEFINED:
            throw std::runtime_error
            ("Convert_tile_format failed due to undefined source format");
        case FORMAT_B8G8R8:
        case FORMAT_R8G8B8:
            s_bpp = 3;
            break;
        case FORMAT_B8G8R8A8:
        case FORMAT_R8G8B8A8:
            s_bpp = 4;
            break;
        default: throw std::runtime_error
            ("Convert_tile_format unsupported bits-per-pixel source format (not 3 or 4 bpp)");
    }
    switch (d_fmt) {
        case FORMAT_UNDEFINED:
            throw std::runtime_error
            ("Convert_tile_format failed due to undefined source format");
        case FORMAT_B8G8R8:
        case FORMAT_R8G8B8:
            d_bpp = 3;
            break;
        case FORMAT_B8G8R8A8:
        case FORMAT_R8G8B8A8:
            d_bpp = 4;
            break;
        default: throw std::runtime_error
            ("Convert_tile_format unsupported bits-per-pixel destination format (not 3 or 4 bpp)");
    } if (!dst || dst->capacity() < TILE_PIX_AREA * d_bpp)
        dst = Create_strong_buffer(TILE_PIX_AREA * d_bpp);
    
    // Task Selection
    // 1) Task number of channels
    switch (s_fmt) {
        case FORMAT_UNDEFINED:
        case FORMAT_B8G8R8:
        case FORMAT_R8G8B8:
            switch (d_fmt) {
                case FORMAT_B8G8R8A8:
                case FORMAT_R8G8B8A8:
                    tasks |= TASK_EXPAND_ALPHA;
                default:break;
            } break;
        case FORMAT_B8G8R8A8:
        case FORMAT_R8G8B8A8:
            switch (d_fmt) {
                case FORMAT_B8G8R8:
                case FORMAT_R8G8B8:
                    tasks |= TASK_STRIP_ALPHA;
                default:break;
            }
    }
    // 2) Task channel ordering
    switch (s_fmt) {
        case FORMAT_UNDEFINED:
        case FORMAT_B8G8R8:
        case FORMAT_B8G8R8A8:
            switch (d_fmt) {
                case FORMAT_R8G8B8:
                case FORMAT_R8G8B8A8:
                    tasks |= TASK_SWAP_0_2;
                default:break;
            } break;
        case FORMAT_R8G8B8:
        case FORMAT_R8G8B8A8:
            switch (d_fmt) {
                case FORMAT_B8G8R8:
                case FORMAT_B8G8R8A8:
                    tasks |= TASK_SWAP_0_2;
                default:break;
            }
    }
    assert(tasks && "Convert_tile_format undefined conversion.");

    // Resizing functions
    assert((tasks & (TASK_EXPAND_ALPHA|TASK_STRIP_ALPHA)) != (TASK_EXPAND_ALPHA|TASK_STRIP_ALPHA) &&
           "Convert_tile_format cannot TASK_EXPAND_ALPHA and TASK_STRIP_ALPHA");
    if (tasks & TASK_EXPAND_ALPHA) {
        HWY_STATIC_DISPATCH(EXPAND_TILE_ADD_ALPHA_8bit)((uint8_t*)src->data(), (uint8_t*)dst->data());
    } else if (tasks & TASK_STRIP_ALPHA) {
        HWY_STATIC_DISPATCH(SHRINK_TILE_RM_ALPHA_8bit)((uint8_t*)src->data(), (uint8_t*)dst->data());
    } else {
        if (src->data() != dst->data())
            memcpy(dst->data(), src->data(), dst->size());
    }
    // byte-swap functions
    if (tasks & TASK_SWAP_0_2) {
        switch (d_bpp) {
            case 3:
                HWY_STATIC_DISPATCH(SWAP_TILE_3_CHANNELS_0_2_8bit)((uint8_t*)dst->data());
                break;
            case 4:
                HWY_STATIC_DISPATCH(SWAP_TILE_4_CHANNELS_0_2_8bit)((uint8_t*)dst->data());
                break;
            default: break;
        }
    }
    
    // Ensure the size is correct before returning
    dst->set_size(TILE_PIX_AREA * d_bpp);
    return dst;
}
namespace N = hwy::HWY_NAMESPACE;
void Downsample_into_tile_2x_avg(const Buffer &src, const Buffer &dst,
                                 uint16_t sub_y, uint16_t sub_x, uint8_t channels)
{
    assert (src->size() <= TILE_PIX_AREA * channels && "Insufficiently sized source tile for 2x downsample");
    assert (dst->size() <= TILE_PIX_AREA * channels && "Insufficiently sized destination tile for 2x downsample");
    switch (channels) {
        case 3: return HWY_STATIC_DISPATCH(DOWNSAMPLE_INTO_TILE_2X_AVG_3)
            (static_cast<uint8_t*>(src->data()),static_cast<uint8_t*>(dst->data()),sub_y, sub_x);
        case 4: return HWY_STATIC_DISPATCH(DOWNSAMPLE_INTO_TILE_2X_AVG_4)
            (static_cast<uint8_t*>(src->data()),static_cast<uint8_t*>(dst->data()),sub_y, sub_x);
        default: throw std::runtime_error("Downsample_into_tile_2x_avg Unsupported channel count");
    }
}
void Downsample_into_tile_4x_avg(const Buffer &src, const Buffer &dst,
                                             uint16_t sub_y, uint16_t sub_x, uint8_t channels)
{
    assert (src->size() <= TILE_PIX_AREA * channels && "Insufficiently sized source tile for 4x downsample");
    assert (dst->size() <= TILE_PIX_AREA * channels && "Insufficiently sized destination tile for 4x downsample");
    switch (channels) {
        case 3: return HWY_STATIC_DISPATCH(DOWNSAMPLE_INTO_TILE_4X_AVG<3>)
            (static_cast<uint8_t*>(src->data()),static_cast<uint8_t*>(dst->data()),sub_y, sub_x);
        case 4: return HWY_STATIC_DISPATCH(DOWNSAMPLE_INTO_TILE_4X_AVG<4>)
            (static_cast<uint8_t*>(src->data()),static_cast<uint8_t*>(dst->data()),sub_y, sub_x);
        default: throw std::runtime_error("Downsample_into_tile_4x_avg Unsupported channel count");
    }
}
} // SIMD
} // Iris
# endif // HWY_ONCE

