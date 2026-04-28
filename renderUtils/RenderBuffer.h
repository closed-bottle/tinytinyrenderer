#ifndef TINYTINYRENDERER_RENDERBUFFER_H
#define TINYTINYRENDERER_RENDERBUFFER_H
#include <cstdint>

#include "SIMD.h"

struct VertexBuffer {
    size_t stride_ = 0;
#ifdef USE_SIMD
    Memory mem_;
    uint8_t offset_ = 0; // offset exists only to support alignment.
#else
    uint8_t* data_ = nullptr;
#endif
    uint64_t count_ = 0;
    uint64_t alloc_count_ = 0; // Size of count that are actually used to allocate for padding.
#ifdef USE_SIMD
    const uint8_t* Data() const {return (mem_.Data() + offset_);}
    uint8_t* Data() {return (mem_.Data() + offset_);}
#else
    constexpr const uint8_t* Data() const {return data_;}
    constexpr uint8_t* Data() {return data_;}
#endif
    VertexBuffer() = delete;
    VertexBuffer(const size_t _stride, void* _data, const uint64_t _vertex_count)
        : stride_(_stride),
#ifdef USE_SIMD
            // Add padding so we can fetch 8 vertices at once + alignment for SIMD.
            mem_(0),
            count_(0)
#else
            data_(static_cast<uint8_t*>(_data)),
            count_(_vertex_count)
#endif
    {

#ifdef USE_SIMD
        // Rearrange them into :
        // Befoer     ->      After
        // X,Y,Z,X,Y,Z->X,X,X,X,X,X,X,X,X

        // count_ is number of vertex,
        // alloc_count is size in byte include padding.
        count_ = _vertex_count;

        // Force alloc_count to be multiple of SIMD vector fetch padding.
        // Simply means that alloc count will be multiple of the number of
        // vectors getting fetched at once.
        alloc_count_ = (_vertex_count - 1) / SIMD_VECTOR_FETCH_PADDING;
        alloc_count_ = (alloc_count_ + 1) * SIMD_VECTOR_FETCH_PADDING;

        // Add SIMD register width per vertex attribute, so we can
        // guarantee to have enough padding.
        mem_ = Memory(stride_ * (alloc_count_ + SIMD_REGISTER_WIDTH));
        offset_ = (SIMD_REGISTER_WIDTH
                - reinterpret_cast<uint64_t>(mem_.Data()) % SIMD_REGISTER_WIDTH);

        // Vec3
        if (_stride == 12) {
            // TODO: There should be better transpose algorithm.

            for (uint64_t i = 0; i < _vertex_count; i += SIMD_VECTOR_FETCH_PADDING) {
                // 1. Load 8 * Vec3, (8 * 12 bytes), think it as 6 * Vec4(which fits into 128 reg.)
                // 2. Merge it into 3 * __m256(3 * 32 bytes) by insert.

                // 1.
                // data_ should be padded to 32 bytes(128 bit).
                auto d256 = (__m128*)(reinterpret_cast<uint64_t>(_data) + (i * _stride));
                __m256 chunk0, chunk1, chunk2;

                // 2.
                chunk0 = _mm256_castps128_ps256(d256[0]);
                chunk1 = _mm256_castps128_ps256(d256[1]);
                chunk2 = _mm256_castps128_ps256(d256[2]);

                chunk0 = _mm256_insertf128_ps(chunk0, d256[3], 1);
                chunk1 = _mm256_insertf128_ps(chunk1, d256[4], 1);
                chunk2 = _mm256_insertf128_ps(chunk2, d256[5], 1);

                auto merged01 = _mm256_shuffle_ps(chunk0, chunk1, _MM_SHUFFLE( 1,0,2,1));
                auto merged23 = _mm256_shuffle_ps(chunk1, chunk2, _MM_SHUFFLE( 2,1,3,2));

                auto x = _mm256_shuffle_ps(chunk0, merged23, _MM_SHUFFLE( 2,0,3,0));
                auto y = _mm256_shuffle_ps(merged01, merged23, _MM_SHUFFLE( 3,1,2,0));
                auto z = _mm256_shuffle_ps(merged01, chunk2, _MM_SHUFFLE( 3,0,3,1));

                memcpy(Data() + sizeof(float) * i, &x, 8 * sizeof(float));
                memcpy(Data() + sizeof(float) * (1 * alloc_count_ + i), &y, 8 * sizeof(float));
                memcpy(Data() + sizeof(float) * (2 * alloc_count_ + i), &z, 8 * sizeof(float));
            }
            const uint64_t last_chunk = _vertex_count
                                - (_vertex_count % SIMD_VECTOR_FETCH_PADDING)
                                + SIMD_VECTOR_FETCH_PADDING;
            // Handle last chunk of data separately, padding exists on the dst,
            // but not in src.
            for (uint64_t i = last_chunk; i < count_; ++i) {
                Lamp::Vec3f* v = (static_cast<Lamp::Vec3f*>(_data) + i);

                memcpy(Data() + i, &v->x, sizeof(float));
                memcpy(Data() + sizeof(float) * (alloc_count_ + i), &v->y, sizeof(float));
                memcpy(Data() + sizeof(float) * (2 * alloc_count_) + i, &v->z, sizeof(float));
            }
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