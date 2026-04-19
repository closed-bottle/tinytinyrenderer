#ifndef TINYTINYRENDERER_IMAGEFORMAT_H
#define TINYTINYRENDERER_IMAGEFORMAT_H
#include <cstdint>

#pragma pack(push, 1)


enum class PixelFormat {
    R8G8B8 = 0,
    B8G8R8,
    D16,
    Count
};

template<PixelFormat f>
struct Texel {
    // Nothing
};

template<>
struct Texel<PixelFormat::R8G8B8>{
    uint8_t R;
    uint8_t G;
    uint8_t B;

    Texel operator+(const Texel& _r) const {
        Texel result;
        result.R += _r.R;
        result.G += _r.G;
        result.B += _r.B;
        return result;
    }

    Texel& operator+=(const Texel& _r) {
        R += _r.R;
        G += _r.G;
        B += _r.B;
        return *this;
    }

    bool operator==(const Texel& _r) const {
        return R == _r.R && G == _r.G && B == _r.B;
    }

    bool operator!=(const Texel& _r) const {
        return R != _r.R || G != _r.G || B != _r.B;
    }
};

template<>
struct Texel<PixelFormat::B8G8R8>{
    uint8_t B;
    uint8_t G;
    uint8_t R;

    Texel operator+(const Texel& _r) const {
        Texel result;
        result.R += _r.R;
        result.G += _r.G;
        result.B += _r.B;
        return result;
    }

    Texel& operator+=(const Texel& _r) {
        R += _r.R;
        G += _r.G;
        B += _r.B;
        return *this;
    }

    bool operator==(const Texel& _r) const {
        return R == _r.R && G == _r.G && B == _r.B;
    }

    bool operator!=(const Texel& _r) const {
        return R != _r.R || G != _r.G || B != _r.B;
    }
};

template<>
struct Texel<PixelFormat::D16>{
    uint16_t D;

    Texel operator+(const Texel& _r) const {
        Texel result;
        result.D += _r.D;
        return result;
    }

    Texel& operator+=(const Texel& _r) {
        D += _r.D;
        return *this;
    }

    bool operator==(const Texel& _r) const {
        return D == _r.D;
    }

    bool operator!=(const Texel& _r) const {
        return D != _r.D;
    }
};
#pragma pack(pop)
#endif //TINYTINYRENDERER_IMAGEFORMAT_H
