#ifndef TINYTINYRENDERER_MESH_H
#define TINYTINYRENDERER_MESH_H
#include "AABB.h++"
#include "special-lamp/lampMath.h++"
#include "special-lamp/lampString.h++"

struct Mesh {
    Lamp::String material_name_;
    Lamp::String object_name_;
    uint64_t vertex_offset_ = 0;
    uint64_t vertex_count_ = 0;
    uint64_t vnormal_offset_ = 0;
    uint64_t vnormal_count_ = 0;
    uint64_t index_offset_ = 0;
    uint64_t index_count_ = 0;
    uint8_t material_offset_ = 0;
    uint8_t material_count_ = 0;

    // AABB
    AABB3 aabb;
};

#endif //TINYTINYRENDERER_MESH_H