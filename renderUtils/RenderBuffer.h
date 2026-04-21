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
            mem_(_stride * _count + SIMD_REGISTER_WIDTH),
#else
            data_(static_cast<uint8_t*>(_data)),
#endif
            count_(_count) {
#ifdef USE_SIMD
        offset_ = reinterpret_cast<uint64_t>(mem_.Data()) % SIMD_REGISTER_WIDTH;
        memcpy( Data(), _data, _stride * _count);
#else
#endif
    }
};

struct IndexBuffer {
    void* data_ = nullptr;
    uint64_t count_ = 0;
};

#endif //TINYTINYRENDERER_RENDERBUFFER_H