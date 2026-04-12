#ifndef TINYTINYRENDERER_IMAGE_H
#define TINYTINYRENDERER_IMAGE_H

#include <vector>
#include "Memory.h"
#include "ImageFormat.h"

using namespace std;

// Developer take responsiblilty for how to handle image after delete.
// It can be reused of destroyed, but using it after freeing memory
// results in UB.
template<PixelFormat F>
class Image {
    Memory& mem_;
    uint32_t width_ = 0;
    uint32_t height_ = 0;
    Texel<F> format_;
    size_t stride_ = 0;
    uint32_t n_pixels_ = 0;
public:
    Image() = delete;
    Image(Memory& _mem, uint32_t _width, uint32_t _height)
        : mem_(_mem), width_(_width), height_(_height), stride_(sizeof(format_)) {
        n_pixels_ = width_ * height_;
    }

    size_t Stride() const { return stride_;}
    uint32_t Width() const { return width_;}
    uint32_t Height() const { return height_;}
    uint32_t NPixels() const { return n_pixels_;}
    uint8_t* Data() const { return mem_.Data(); }
};


#endif //TINYTINYRENDERER_IMAGE_H