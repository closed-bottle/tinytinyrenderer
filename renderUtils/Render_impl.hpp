#include "Render.h"
#include "RenderCmd.h"


namespace {
#ifdef USE_SIMD
    void MatrixVectorMul(const __m128& _c0, const __m128& _c1, const __m128& _c2, const __m128& _c3,
                         const float* _x, const float* _y, const float* _z, const float* _w,
                         float* _out) {
        const __m128 vv0 = _mm_load_ps(_x);
        const __m128 vv1 = _mm_load_ps(_y);
        const __m128 vv2 = _mm_load_ps(_z);
        const __m128 vv3 = _mm_load_ps(_w);

        __m128 result = _mm_set_ps(0,0,0,0);

        result = _mm_fmadd_ps(_c0, vv0, result);
        result = _mm_fmadd_ps(_c1, vv1, result);
        result = _mm_fmadd_ps(_c2, vv2, result);
        result = _mm_fmadd_ps(_c3, vv3, result);

        _mm_store_ps(_out, result);
    }
#endif
    template<typename T>
    const T* SampleData(const uint8_t* _data, const uint64_t& _offset, const uint64_t& _i) {
        return reinterpret_cast<const T*>(_data + _offset + (sizeof(T) * _i));
    }

    void ClipSpaceScreenSpace(const RenderCmdInfo& _cmd_info, const Lamp::Mat4f &_viewport, Lamp::Vec4f& _v) {
        _v /= _v.w;
        _v = _viewport * _v;
    }

    // Note that it is not the proper algorithm to plot points on the screen,
    // it is unstable due to the nature direct casting.
    // Only for quick demonstration.
    void DrawPointShader(const RenderCmdInfo& _cmd_info) {
        // upper left = origin.
        auto& render_target = _cmd_info.render_info_->_color_att->image_;
        const auto& uniform = static_cast<const Render::UMvp*>(_cmd_info.uniform_);
        auto& view_port = _cmd_info.view_port_;

        const uint32_t uiwidth = view_port->width;
        const uint32_t uiheight = view_port->height;

        const float f_width  = view_port->width;
        const float f_height = view_port->height;

        const Lamp::Mat4f viewport_transform
            = Lamp::Mat4f::Translate(view_port->x + f_width * .5f,
                                     view_port->y + f_height * .5f, (view_port->far + view_port->near) / 2.0f)
            * Lamp::Mat4f::Scale(f_width * .5f, f_height * -.5f, (view_port->far - view_port->near) / 2.0f);


        for (uint64_t i = 0; i < _cmd_info.vertex_buffer_->count_; ++i) {
            auto v3 = *(reinterpret_cast<const Lamp::Vec3f*>(_cmd_info.vertex_buffer_->Data()) + (sizeof(Lamp::Vec3f) * i));
            Lamp::Vec4f v4 = {v3.x, v3.y, v3.z, 1};

            v4 = uniform->mvp * v4;

            ClipSpaceScreenSpace(_cmd_info, viewport_transform, v4);

            constexpr uint8_t color[] = {255, 0, 255};
            if (v4.x >= view_port->x
             && v4.x < view_port->x + uiwidth
             && v4.y >= view_port->y
             && v4.y < view_port->y + uiheight) {
                void* ptr = static_cast<uint8_t *>(render_target.Data())
                            + uiwidth * (uint32_t)v4.y + (uint32_t)v4.x;
                memcpy(ptr, color, render_target.Stride());
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

    void plotLine(const RenderCmdInfo& _cmd_info, const Lamp::Vec4f& _start, const Lamp::Vec4f& _end) {
        constexpr uint8_t color[] = {255, 255, 0};
        auto& render_target = _cmd_info.render_info_->_color_att->image_;

        auto& view_port = _cmd_info.view_port_;
        const int x = view_port->x;
        const int y = view_port->y;
        const uint32_t uiwidth = view_port->width;
        const uint32_t uiheight = view_port->height;

        int x0 = _start.x;
        int x1 = _end.x;
        int y0 = _start.y;
        int y1 = _end.y;


        int dx = abs(x1-x0), sx = x0<x1 ? 1 : -1;
        int dy = -abs(y1-y0), sy = y0<y1 ? 1 : -1;
        int err = dx+dy, e2; /* error value e_xy */
        for (;;){ /* loop */
            if (x0 >= x && x0 < (x + uiwidth) && y0 >= y && y0 < (y + uiheight)) {
                void* ptr = static_cast<uint8_t *>(render_target.Data())
                            + ((render_target.Width() * y0 + x0) * render_target.Stride());
                memcpy(ptr, color, render_target.Stride());
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
    void DrawTriangleLineShader(const RenderCmdInfo& _cmd_info) {
        auto* vertices = reinterpret_cast<const Lamp::Vec3f*>(_cmd_info.vertex_buffer_->Data());
        auto& render_target = _cmd_info.render_info_->_color_att->image_;
        auto& vertex_buffer = _cmd_info.vertex_buffer_;
        auto& index_buffer = _cmd_info.index_buffer_;
        const auto& uniform = static_cast<const Render::UMvp*>(_cmd_info.uniform_);

        auto& view_port = _cmd_info.view_port_;
        const float f_width  = view_port->width;
        const float f_height = view_port->height;

        const Lamp::Mat4f viewport_transform
            = Lamp::Mat4f::Translate(view_port->x + f_width * .5f,
                                     view_port->y + f_height * .5f, (view_port->far + view_port->near) / 2.0f)
            * Lamp::Mat4f::Scale(f_width * .5f, f_height * -.5f, (view_port->far - view_port->near) / 2.0f);

        // TODO: matrix multiplication duplicated on same vertices(indexed.)
        // TODO: maybe don't need to apply mvp across every vertices in buffer,
        // since index may refer to only some of the vertices in buffer.
#ifdef USE_SIMD
        __m128 c0 = _mm_load_ps((float*)&uniform->mvp.c0);
        __m128 c1 = _mm_load_ps((float*)&uniform->mvp.c1);
        __m128 c2 = _mm_load_ps((float*)&uniform->mvp.c2);
        __m128 c3 = _mm_load_ps((float*)&uniform->mvp.c3);

        // Few things to consider :
        // SIMD version will use more memory footprint because it uses pre-processed vertex.
        // It might need alloc-dealloc every draw, so might be good choice to just allocating it
        // during initialize.

        // alignment and offset is already calculated for vertex.
        Memory preprocess = Memory(_cmd_info.vertex_buffer_->count_);

        // TODO: Don't forget to -1 and handle padding at the end.
        for (uint64_t i = _cmd_info.first_index_; i < index_buffer->count_; i += 8) {
            uint32_t index[8] = {};
            alignas(16) Lamp::Vec4f v_simd[8] = {};
            for (uint8_t j = 0; j < 8; ++j) {
                index[j] = *(static_cast<uint32_t*>(index_buffer->data_) + j);
                v_simd[j] = Lamp::Vec4f(vertices[index[j]].x, vertices[index[j]].y, vertices[index[j]].z, 1.0f);
            }

            //MatrixVectorMul(c0, c1, c2, c3, v0);
            //MatrixVectorMul(c0, c1, c2, c3, v1);
            //MatrixVectorMul(c0, c1, c2, c3, v2);

            for (uint8_t j = 0; j < 8; ++j) {
                ClipSpaceScreenSpace(_cmd_info, viewport_transform, v_simd[j]);
            }

            //plotLine(_cmd_info, v0, v2);
            //plotLine(_cmd_info, v2, v1);
            //plotLine(_cmd_info, v1, v0);
        }
#else
        Memory preprocess = Memory(vertex_buffer->count_ * sizeof(Lamp::Vec4f));

        uint32_t start = 0;
        uint32_t end = 0;
        for (uint64_t i = _cmd_info.first_index_; i < index_buffer->count_; ++i) {
            start = std::min(start, *(static_cast<uint32_t*>(index_buffer->data_) + i));
            end = std::max(end, *(static_cast<uint32_t*>(index_buffer->data_) + i));
        }

        for (uint64_t i = start; i <= end; ++i) {
            alignas(16) Lamp::Vec4f v0 = Lamp::Vec4f(vertices[i].x, vertices[i].y, vertices[i].z, 1.0f);
            v0 = uniform->mvp * v0;
            ClipSpaceScreenSpace(_cmd_info, viewport_transform, v0);

            memcpy(preprocess.Data() + (i * sizeof(Lamp::Vec4f)), &v0, sizeof(Lamp::Vec4f));
        }

        auto new_verticecs = reinterpret_cast<const Lamp::Vec4f*>(preprocess.Data());

        for (uint64_t i = 0; i < index_buffer->count_; i += 3) {
            auto i0 = *(static_cast<uint32_t*>(index_buffer->data_) + i);
            auto i1 = *(static_cast<uint32_t*>(index_buffer->data_) + i+1);
            auto i2 = *(static_cast<uint32_t*>(index_buffer->data_) + i+2);

            alignas(16) Lamp::Vec4f v0 = new_verticecs[i0];
            alignas(16) Lamp::Vec4f v1 = new_verticecs[i1];
            alignas(16) Lamp::Vec4f v2 = new_verticecs[i2];

            plotLine(_cmd_info, v0, v2);
            plotLine(_cmd_info, v2, v1);
            plotLine(_cmd_info, v1, v0);
        }
#endif

    }
}


void Render::Draw(const RenderCmdInfo& _cmd_info) {
    switch (_cmd_info.uniform_->sType) {
        case ShaderName::PointShader:
            DrawPointShader(_cmd_info);
            break;
        case ShaderName::LineShader:
            DrawTriangleLineShader(_cmd_info);
            break;
        case ShaderName::Count:
            // Not implemented
            break;
    }
}