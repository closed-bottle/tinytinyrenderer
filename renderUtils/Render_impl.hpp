#include "Render.h"

namespace {

    template<typename T>
    const T* SampleData(const uint8_t* _data, const uint64_t& _offset, const uint64_t& _i) {
        return reinterpret_cast<const T*>(_data + _offset + (sizeof(T) * _i));
    }

    template<PixelFormat PF>
    void ClipSpaceScreenSpace(Image<PF>& _render_target, Lamp::Vec4f& _v) {
        _v /= _v.w;

        // Viewport transform.
        _v.x *= _render_target.Width();
        _v.y *= _render_target.Height();
        _v.x += _render_target.Width()/2;
        _v.y += _render_target.Height()/2;
    }

    // Note that it is not the proper algorithm to plot points on the screen,
    // it is unstable due to the nature direct casting.
    // Only for quick demonstration.
    template<PixelFormat PF>
    void DrawPointShader(Image<PF>& _render_target, const Geometry& _geom, const Mesh& _mesh,
                         const Render::UMvp* _uniform) {
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
                _render_target.Data()[_render_target.Width() * (uint32_t)v4.y + (uint32_t)v4.x] = {255, 0, 255};
            }
        }
    }

    // https://en.wikipedia.org/wiki/Bresenham's_line_algorithm
    // https://zingl.github.io/bresenham.html
    // Implementation based on error increment.
    // This implementation is based on the paper released by Alois Zingl,
    // "A Rasterizing Algorithm for Drawing Curves".
    // Copyright (c) Alois Zingl
    // The code (function "plotLine") Licensed under the MIT License
    template<PixelFormat PF>
    void plotLine(Image<PF>& _render_target, const Lamp::Vec4f& _start, const Lamp::Vec4f& _end) {

        int x0 = _start.x;
        int x1 = _end.x;
        int y0 = _start.y;
        int y1 = _end.y;


        int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
        int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
        int err = dx+dy, e2; /* error value e_xy */
        for (;;){ /* loop */
            if (x0 > 0 && x0 < _render_target.Width() && y0 > 0 && y0 < _render_target.Height()) {
                _render_target.Data()[_render_target.Width() * y0 + x0] = {255, 255, 0};
            }
            e2 = 2*err;
            if (e2 >= dy) { /* e_xy+e_x > 0 */
                if (x0 == x1) break;
                err += dy; x0 += sx;
            }
            if (e2 <= dx) { /* e_xy+e_y < 0 */
                if (y0 == y1) break;
                err += dx; y0 += sy;
            }
        }
    }

    // Assume primitive is always triangle strip.
    // It can be added to template later if needed to implement other primitives.
    template<PixelFormat PF>
    void DrawTriangleLineShader(Image<PF>& _render_target, const Geometry& _geom, const Mesh& _mesh,
                         const Render::UMvp* _uniform) {

        const auto* vertices = SampleData<Lamp::Vec3f>(_geom.vertex_.Data(), _mesh.vertex_offset_, 0);

        for (uint64_t i = 0; i < _mesh.index_count_; i += 3) {
            auto i0 = *SampleData<uint32_t>(_geom.index_.Data(), _mesh.index_offset_, i);
            auto i1 = *SampleData<uint32_t>(_geom.index_.Data(), _mesh.index_offset_, i+1);
            auto i2 = *SampleData<uint32_t>(_geom.index_.Data(), _mesh.index_offset_, i+2);

            Lamp::Vec4f v0 = _uniform->mvp * Lamp::Vec4f(vertices[i0].x, vertices[i0].y, vertices[i0].z, 1.0f);
            Lamp::Vec4f v1 = _uniform->mvp * Lamp::Vec4f(vertices[i1].x, vertices[i1].y, vertices[i1].z, 1.0f);
            Lamp::Vec4f v2 = _uniform->mvp * Lamp::Vec4f(vertices[i2].x, vertices[i2].y, vertices[i2].z, 1.0f);

            ClipSpaceScreenSpace(_render_target, v0);
            ClipSpaceScreenSpace(_render_target, v1);
            ClipSpaceScreenSpace(_render_target, v2);

            // TODO : exclude already rendered lines.
            auto l0 = v2 - v0;
            auto l1 = v1 - v2;
            auto l2 = v0 - v1;

            plotLine(_render_target, v0, v2);
            plotLine(_render_target, v2, v1);
            plotLine(_render_target, v1, v0);
        }
    }
}

template<PixelFormat PF>
void Render::Draw(Image<PF>& _render_target, const Geometry& _geom,
          const Mesh& _mesh, const ShaderFootprint* _uniform) {
    switch (_uniform->sType) {
        case ShaderName::PointShader:
            DrawPointShader(_render_target, _geom, _mesh,
                reinterpret_cast<const Render::UMvp*>(_uniform));
            break;
        case ShaderName::LineShader:
            DrawTriangleLineShader(_render_target, _geom, _mesh,
                reinterpret_cast<const Render::UMvp*>(_uniform));
            break;
    }
}