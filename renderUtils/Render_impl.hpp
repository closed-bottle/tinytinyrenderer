#include "Render.h"

namespace {

    // Note that it is not the proper algorithm to plot points on the screen,
    // it is unstable due to the nature direct casting.
    // Only for quick demonstration.
    template<PixelFormat PF>
    void DrawPointShader(Image<PF>& _render_target, const Geometry& _geom, const Mesh& _mesh,
                         const Render::UPointShader* _uniform) {
        // upper left = origin.
        for (uint64_t i = 0; i < _mesh.vertex_count_; ++i) {
            auto v3 = *reinterpret_cast<const Lamp::Vec3f*>(_geom.vertex_.Data() + _mesh.vertex_offset_ + (sizeof(Lamp::Vec3f) * i));
            Lamp::Vec4f v4 = {v3.x, v3.y, v3.z, 1};

            v4 = _uniform->mvp * v4;

            v4 /= v4.w;
            v4.x *= _render_target.Width();
            v4.y *= _render_target.Height();
            v4.x += _render_target.Width()/2;
            v4.y += _render_target.Height()/2;


            if (v4.x > 0 && v4.x < _render_target.Width() && v4.y > 0 && v4.y < _render_target.Height()) {
                _render_target.Data()[_render_target.Width() * (uint32_t)v4.y + (uint32_t)v4.x] = {0, 0, 255};
            }
        }
    }
}

template<PixelFormat PF>
void Render::Draw(Image<PF>& _render_target, const Geometry& _geom,
          const Mesh& _mesh, const ShaderFootprint* _uniform) {
    if (_uniform->sType == ShaderName::PointShader)
        DrawPointShader(_render_target, _geom, _mesh,
            reinterpret_cast<const Render::UPointShader*>(_uniform));
}