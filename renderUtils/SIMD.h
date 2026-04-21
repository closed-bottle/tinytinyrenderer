#ifndef SWRENDERER_SIMD_H
#define SWRENDERER_SIMD_H

#ifdef USE_SIMD
#include <immintrin.h>
constexpr size_t SIMD_REGISTER_WIDTH = 32;
#endif
#endif //SWRENDERER_SIMD_H