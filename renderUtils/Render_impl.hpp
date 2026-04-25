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

        auto& render_target = _cmd_info.render_info_->_color_att->image_;
        auto& vertex_buffer = _cmd_info.vertex_buffer_;
        auto& index_buffer = _cmd_info.index_buffer_;
        const auto& uniform = static_cast<const Render::UMvp*>(_cmd_info.uniform_);

        auto& view_port = _cmd_info.view_port_;
        const float f_width  = view_port->width;
        const float f_height = view_port->height;

        Lamp::Mat4f viewport_transform
            = Lamp::Mat4f::Translate(view_port->x + f_width * .5f,
                                     view_port->y + f_height * .5f, (view_port->far + view_port->near) / 2.0f)
            * Lamp::Mat4f::Scale(f_width * .5f, f_height * -.5f, (view_port->far - view_port->near) / 2.0f);

        //viewport_transform = viewport_transform * uniform->mvp;
        //viewport_transform = uniform->mvp;
        // TODO: matrix multiplication duplicated on same vertices(indexed.)
        // TODO: maybe don't need to apply mvp across every vertices in buffer,
        // since index may refer to only some of the vertices in buffer.
#ifdef USE_SIMD
        // Few things to consider :
        // SIMD version will use more memory footprint because it uses pre-processed vertex.
        // It might need alloc-dealloc every draw, so might be good choice to just allocating it
        // during initialize.

        // alignment and offset is already calculated for vertex.
        Memory preprocess = Memory(vertex_buffer->count_ * sizeof(Lamp::Vec4f));

        alignas(SIMD_REGISTER_WIDTH) const float ws[8] = { 1, 1, 1, 1, 1, 1, 1, 1};

        // TODO : Maybe just do it while loading files.
        uint32_t start = 0;
        uint32_t end = 0;
        for (uint64_t i = _cmd_info.first_index_; i < index_buffer->count_; ++i) {
            start = std::min(start, *(static_cast<uint32_t*>(index_buffer->data_) + i));
            end = std::max(end, *(static_cast<uint32_t*>(index_buffer->data_) + i));
        }

        uint32_t count = index_buffer->count_ ? (end - start) + 1 : 0;
        count -= (count % SIMD_VECTOR_FETCH_PADDING);
        // Exclude last 8 elements, so we don't use padded value.

        Lamp::Mat4f merged_mat = viewport_transform * uniform->mvp;

        __m256 merged[16] = {
            _mm256_broadcast_ss(&merged_mat.c0.x),
            _mm256_broadcast_ss(&merged_mat.c1.x),
            _mm256_broadcast_ss(&merged_mat.c2.x),
            _mm256_broadcast_ss(&merged_mat.c3.x),
            _mm256_broadcast_ss(&merged_mat.c0.y),
            _mm256_broadcast_ss(&merged_mat.c1.y),
            _mm256_broadcast_ss(&merged_mat.c2.y),
            _mm256_broadcast_ss(&merged_mat.c3.y),
            _mm256_broadcast_ss(&merged_mat.c0.z),
            _mm256_broadcast_ss(&merged_mat.c1.z),
            _mm256_broadcast_ss(&merged_mat.c2.z),
            _mm256_broadcast_ss(&merged_mat.c3.z),
            _mm256_broadcast_ss(&merged_mat.c0.w),
            _mm256_broadcast_ss(&merged_mat.c1.w),
            _mm256_broadcast_ss(&merged_mat.c2.w),
            _mm256_broadcast_ss(&merged_mat.c3.w)
        };

        auto* xs
            = reinterpret_cast<const float*>(_cmd_info.vertex_buffer_->Data());
        auto* ys
            = reinterpret_cast<const float*>(_cmd_info.vertex_buffer_->Data()) + 1*_cmd_info.vertex_buffer_->alloc_count_;
        auto* zs
            = reinterpret_cast<const float*>(_cmd_info.vertex_buffer_->Data()) + 2*_cmd_info.vertex_buffer_->alloc_count_;
        __m256 ww = _mm256_load_ps(ws);

        uint64_t j = 0;
        for (uint64_t i = start; i < end && j < count; i += 8, j += 8) {
            // Need more test on alignment, only tested with 2 meshes.
            // Aligned with SIMD_REGISTER_WIDTH.
            __m256 xx = _mm256_load_ps(&xs[i]);
            __m256 yy = _mm256_load_ps(&ys[i]);
            __m256 zz = _mm256_load_ps(&zs[i]);


            __m256* x = reinterpret_cast<__m256*>(preprocess.Data()
                            + sizeof(float) * j);
            __m256* y = reinterpret_cast<__m256*>(preprocess.Data()
                            + sizeof(float) * ((1* vertex_buffer->count_) + j));
            __m256* z = reinterpret_cast<__m256*>(preprocess.Data()
                            + sizeof(float) * ((2* vertex_buffer->count_) + j));

            *x = _mm256_mul_ps(merged[0], xx);
            *x = _mm256_fmadd_ps(merged[1], yy, *x);
            *x = _mm256_fmadd_ps(merged[2], zz, *x);
            *x = _mm256_fmadd_ps(merged[3], ww, *x);

            *y = _mm256_mul_ps(merged[4], xx);
            *y = _mm256_fmadd_ps(merged[5], yy, *y);
            *y = _mm256_fmadd_ps(merged[6], zz, *y);
            *y = _mm256_fmadd_ps(merged[7], ww, *y);

            *z = _mm256_mul_ps(merged[8], xx);
            *z = _mm256_fmadd_ps(merged[9], yy, *z);
            *z = _mm256_fmadd_ps(merged[10], zz, *z);
            *z = _mm256_fmadd_ps(merged[11], ww, *z);

            __m256 clip_w;
            clip_w = _mm256_mul_ps(merged[12], xx);
            clip_w = _mm256_fmadd_ps(merged[13], yy, clip_w);
            clip_w = _mm256_fmadd_ps(merged[14], zz, clip_w);
            clip_w = _mm256_fmadd_ps(merged[15], ww, clip_w);

            _mm256_div_ps(*x, clip_w);
            _mm256_div_ps(*y, clip_w);
            _mm256_div_ps(*z, clip_w);
        }


        for (j; j < vertex_buffer->count_; ++j) {
            alignas(16) Lamp::Vec4f v0 = Lamp::Vec4f(xs[j], ys[j], zs[j], 1);
            v0 = uniform->mvp * v0;
            ClipSpaceScreenSpace(_cmd_info, viewport_transform, v0);


            memcpy(preprocess.Data() + sizeof(float) * j, &v0.x, sizeof(float));
            memcpy(preprocess.Data() + sizeof(float) * (1* vertex_buffer->count_ + j), &v0.y, sizeof(float));
            memcpy(preprocess.Data() + sizeof(float) * (2* vertex_buffer->count_ + j), &v0.z, sizeof(float));
            memcpy(preprocess.Data() + sizeof(float) * (3* vertex_buffer->count_ + j), &v0.w, sizeof(float));
        }



        auto i_d = static_cast<uint8_t*>(index_buffer->data_);
        const auto* x = reinterpret_cast<const float*>(preprocess.Data());
        const auto* y = reinterpret_cast<const float*>(preprocess.Data() + sizeof(float) * (1 * vertex_buffer->count_));
        const auto* z = reinterpret_cast<const float*>(preprocess.Data() + sizeof(float) * (2 * vertex_buffer->count_));
        const auto* w = reinterpret_cast<const float*>(preprocess.Data() + sizeof(float) * (3 * vertex_buffer->count_));

        for (uint64_t i = 0; i < index_buffer->count_; i += 3) {
            auto i0 = *reinterpret_cast<uint32_t*>(i_d + sizeof(uint32_t) * i);
            auto i1 = *reinterpret_cast<uint32_t*>(i_d + sizeof(uint32_t) * (i+1));
            auto i2 = *reinterpret_cast<uint32_t*>(i_d + sizeof(uint32_t) * (i+2));

            alignas(16) Lamp::Vec4f v0 = {x[i0], y[i0], z[i0], w[i0]};
            alignas(16) Lamp::Vec4f v1 = {x[i1], y[i1], z[i1], w[i1]};
            alignas(16) Lamp::Vec4f v2 = {x[i2], y[i2], z[i2], w[i2]};

            plotLine(_cmd_info, v0, v2);
            plotLine(_cmd_info, v2, v1);
            plotLine(_cmd_info, v1, v0);
        }

#else
        auto* vertices = reinterpret_cast<const Lamp::Vec3f*>(_cmd_info.vertex_buffer_->Data());
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