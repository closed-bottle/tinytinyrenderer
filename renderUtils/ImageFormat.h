#ifndef TINYTINYRENDERER_IMAGEFORMAT_H
#define TINYTINYRENDERER_IMAGEFORMAT_H
#include <cstdint>

#pragma pack(push, 1)


enum class PixelFormat {
    R8G8B8 = 0,
    Count
};

template<PixelFormat f>
struct ImageFormat {
    // Nothing
};

template<>
struct ImageFormat<PixelFormat::R8G8B8>{
    uint8_t R;
    uint8_t G;
    uint8_t B;
};
#pragma pack(pop)
#endif //TINYTINYRENDERER_IMAGEFORMAT_H
