#ifndef TINYTINYRENDERER_RENDERBUFFER_H
#define TINYTINYRENDERER_RENDERBUFFER_H
#include <cstdint>

struct VertexBuffer {
    void* data_ = nullptr;
    uint64_t count_ = 0;
};


struct IndexBuffer {
    void* data_ = nullptr;
    uint64_t count_ = 0;
};

#endif //TINYTINYRENDERER_RENDERBUFFER_H