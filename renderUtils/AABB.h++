#ifndef TINYTINYRENDERER_AABB_H
#define TINYTINYRENDERER_AABB_H
#include <cstdint>

#include "special-lamp/lampMath.h++"

template <uint8_t n_elements>
struct AABB {
};

struct AABB2 : public AABB<2> {
        Lamp::Vec2f min;
        Lamp::Vec2f max;
};

struct AABB2i : public AABB<2> {
        Lamp::Vec2i min;
        Lamp::Vec2i max;
};

struct AABB3 : public AABB<3> {
        Lamp::Vec3f min;
        Lamp::Vec3f max;
};

#endif //TINYTINYRENDERER_AABB_H