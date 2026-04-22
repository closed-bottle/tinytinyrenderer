#ifndef TINYTINYRENDERER_RENDERBUFFER_H
#define TINYTINYRENDERER_RENDERBUFFER_H
#include <cstdint>

struct VertexBuffer {
    size_t stride_ = 0;
#ifdef USE_SIMD
    Memory mem_;
    uint8_t offset_ = 0; // offset exists only to support alignment.
#else
    uint8_t* data_ = nullptr;
#endif
    uint64_t count_ = 0;

#ifdef USE_SIMD
    constexpr const uint8_t* Data() const {return (mem_.Data() + offset_);}
    constexpr uint8_t* Data() {return (mem_.Data() + offset_);}
#else
    constexpr const uint8_t* Data() const {return data_;}
    constexpr uint8_t* Data() {return data_;}
#endif
    VertexBuffer() = delete;
    VertexBuffer(const size_t _stride, void* _data, const uint64_t _count)
        : stride_(_stride),
#ifdef USE_SIMD
            // Adding padding to match count%8 == 0
            mem_(_stride * (_count + (8 - (_count % 8))) + SIMD_REGISTER_WIDTH),
#else
            data_(static_cast<uint8_t*>(_data)),
#endif
            count_(_count) {

#ifdef USE_SIMD
        offset_ = reinterpret_cast<uint64_t>(mem_.Data()) % SIMD_REGISTER_WIDTH;
        // Rearrange them into :
        // Befoer     ->      After
        // X,Y,Z,X,Y,Z->X,X,X,X,X,X,X,X,X


        // Vec3
        if (_stride == 12) {
            // 1. Load 8 * Vec3, (8 * 12 bytes), think it as 6 * Vec4(which fits into 128 reg.)
            // 2. Merge it into 3 * __m256(3 * 32 bytes) by insert.

            // 1.
            __m128* d256 = (__m128*)(_data); // data_ should be padded to 32 bytes(128 bit).
            __m256 chunk0, chunk1, chunk2;

            // 2.
            chunk0 = _mm256_insertf128_ps(chunk0, d256[0], 1);
            chunk1 = _mm256_insertf128_ps(chunk0, d256[1], 1);
            chunk2 = _mm256_insertf128_ps(chunk0, d256[2], 1);

            chunk0 = _mm256_insertf128_ps(chunk0, d256[3], 1);
            chunk1 = _mm256_insertf128_ps(chunk0, d256[4], 1);
            chunk2 = _mm256_insertf128_ps(chunk0, d256[5], 1);
        }





#else
#endif
    }
};

struct IndexBuffer {
    void* data_ = nullptr;
    uint64_t count_ = 0;
};

#endif //TINYTINYRENDERER_RENDERBUFFER_H