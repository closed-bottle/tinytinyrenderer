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

    void ClipToNdc(Lamp::Vec4f &_v) {
        _v /= _v.w;
    }

    void NdcToWindow(const Lamp::Mat4f &_viewport, Lamp::Vec4f &_v) {
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
            ClipToNdc(v4);
            NdcToWindow(viewport_transform, v4);


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

#ifdef USE_SIMD
        // Few things to consider :
        // SIMD version will use more memory footprint because it uses pre-processed vertex.
        // It might need alloc-dealloc every draw, so might be good choice to just allocating it
        // during initialize.

        // alignment and offset is already calculated for vertex.
        auto preprocess_size = sizeof(Lamp::Vec4f) * (vertex_buffer->alloc_count_ + SIMD_REGISTER_WIDTH);
        auto preprocess = Memory(preprocess_size);
        memset(preprocess.Data(), 0, preprocess_size);
        uint64_t offset = (SIMD_REGISTER_WIDTH - reinterpret_cast<uint64_t>(preprocess.Data()) % SIMD_REGISTER_WIDTH);
        auto raster_data = &preprocess.Data()[offset];

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

        auto* in_x
            = reinterpret_cast<const float*>(_cmd_info.vertex_buffer_->Data());
        auto* in_y
            = reinterpret_cast<const float*>(_cmd_info.vertex_buffer_->Data())
                + 1 *_cmd_info.vertex_buffer_->alloc_count_;
        auto* in_z
            = reinterpret_cast<const float*>(_cmd_info.vertex_buffer_->Data())
                + 2 *_cmd_info.vertex_buffer_->alloc_count_;
        __m256 ww = _mm256_load_ps(ws);

        uint64_t j = 0;
        for (uint64_t i = start; i < end && j < count; i += 8, j += 8) {
            // Need more test on alignment, only tested with 2 meshes.
            // Aligned with SIMD_REGISTER_WIDTH.
            __m256 xx = _mm256_load_ps(&in_x[i]);
            __m256 yy = _mm256_load_ps(&in_y[i]);
            __m256 zz = _mm256_load_ps(&in_z[i]);

            alignas(32) __m256 out_x;
            alignas(32) __m256 out_y;
            alignas(32) __m256 out_z;

            out_x = _mm256_mul_ps(merged[0], xx);
            out_x = _mm256_fmadd_ps(merged[1], yy, out_x);
            out_x = _mm256_fmadd_ps(merged[2], zz, out_x);
            out_x = _mm256_fmadd_ps(merged[3], ww, out_x);

            out_y = _mm256_mul_ps(merged[4], xx);
            out_y = _mm256_fmadd_ps(merged[5], yy, out_y);
            out_y = _mm256_fmadd_ps(merged[6], zz, out_y);
            out_y = _mm256_fmadd_ps(merged[7], ww, out_y);

            out_z = _mm256_mul_ps(merged[8], xx);
            out_z = _mm256_fmadd_ps(merged[9], yy, out_z);
            out_z = _mm256_fmadd_ps(merged[10], zz, out_z);
            out_z = _mm256_fmadd_ps(merged[11], ww, out_z);

            __m256 clip_w;
            clip_w = _mm256_mul_ps(merged[12], xx);
            clip_w = _mm256_fmadd_ps(merged[13], yy, clip_w);
            clip_w = _mm256_fmadd_ps(merged[14], zz, clip_w);
            clip_w = _mm256_fmadd_ps(merged[15], ww, clip_w);

            out_x = _mm256_div_ps(out_x, clip_w);
            out_y = _mm256_div_ps(out_y, clip_w);
            out_z = _mm256_div_ps(out_z, clip_w);

            memcpy(&raster_data[sizeof(float) * j], &out_x, sizeof(__m256));
            memcpy(&raster_data[sizeof(float) * ((1 * vertex_buffer->alloc_count_) + j)], &out_y, sizeof(__m256));
            memcpy(&raster_data[sizeof(float) * ((2 * vertex_buffer->alloc_count_) + j)], &out_x, sizeof(__m256));
        }


        for (; j < vertex_buffer->count_; ++j) {
            alignas(16) Lamp::Vec4f v0 = Lamp::Vec4f(in_x[j], in_y[j], in_z[j], 1);
            v0 = uniform->mvp * v0;
            ClipSpaceScreenSpace(viewport_transform, v0);


            memcpy(&raster_data[sizeof(float) * j], &v0.x, sizeof(float));
            memcpy(&raster_data[sizeof(float) * (1* vertex_buffer->alloc_count_ + j)], &v0.y, sizeof(float));
            memcpy(&raster_data[sizeof(float) * (2* vertex_buffer->alloc_count_ + j)], &v0.z, sizeof(float));
            memcpy(&raster_data[sizeof(float) * (3* vertex_buffer->alloc_count_ + j)], &v0.w, sizeof(float));
        }



        auto i_d = static_cast<uint8_t*>(index_buffer->data_);

        uint8_t* x = raster_data;
        uint8_t* y = &raster_data[sizeof(float) * (1 * vertex_buffer->alloc_count_)];
        uint8_t* z = &raster_data[sizeof(float) * (2 * vertex_buffer->alloc_count_)];
        uint8_t* w = &raster_data[sizeof(float) * (3 * vertex_buffer->alloc_count_)];

        for (uint64_t i = 0; i < index_buffer->count_; i += 3) {
            uint32_t i0, i1, i2;

            memcpy(&i0, &i_d[sizeof(uint32_t) * (i+0)], sizeof(uint32_t));
            memcpy(&i1, &i_d[sizeof(uint32_t) * (i+1)], sizeof(uint32_t));
            memcpy(&i2, &i_d[sizeof(uint32_t) * (i+2)], sizeof(uint32_t));

            alignas(16) Lamp::Vec4f v0, v1, v2;
            memcpy(&v0.x, &x[sizeof(float) * i0], sizeof(float));
            memcpy(&v0.y, &y[sizeof(float) * i0], sizeof(float));
            memcpy(&v0.z, &z[sizeof(float) * i0], sizeof(float));
            memcpy(&v0.w, &w[sizeof(float) * i0], sizeof(float));

            memcpy(&v1.x, &x[sizeof(float) * i1], sizeof(float));
            memcpy(&v1.y, &y[sizeof(float) * i1], sizeof(float));
            memcpy(&v1.z, &z[sizeof(float) * i1], sizeof(float));
            memcpy(&v1.w, &w[sizeof(float) * i1], sizeof(float));

            memcpy(&v2.x, &x[sizeof(float) * i2], sizeof(float));
            memcpy(&v2.y, &y[sizeof(float) * i2], sizeof(float));
            memcpy(&v2.z, &z[sizeof(float) * i2], sizeof(float));
            memcpy(&v2.w, &w[sizeof(float) * i2], sizeof(float));

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
            ClipToNdc(v0);
            NdcToWindow(viewport_transform, v0);

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


    void DrawRasterShader(const RenderCmdInfo& _cmd_info) {
#ifdef USE_SIMD
#else
        auto& vertex_buffer = _cmd_info.vertex_buffer_;
        auto& index_buffer = _cmd_info.index_buffer_;
        const auto& uniform = dynamic_cast<const Render::UMvp*>(_cmd_info.uniform_);

        auto& view_port = _cmd_info.view_port_;
        const float f_width  = view_port->width;
        const float f_height = view_port->height;

        Lamp::Mat4f viewport_transform
            = Lamp::Mat4f::Translate(view_port->x + f_width * .5f,
                                     view_port->y + f_height * .5f, (view_port->far + view_port->near) / 2.0f)
            * Lamp::Mat4f::Scale(f_width * .5f, f_height * -.5f, (view_port->far - view_port->near) / 2.0f);

        auto* vertices = reinterpret_cast<const Lamp::Vec3f*>(_cmd_info.vertex_buffer_->Data());
        auto preprocess = Memory(vertex_buffer->count_ * sizeof(Lamp::Vec4f));
        auto raster_data = preprocess.Data();

        uint32_t start = 0;
        uint32_t end = 0;
        for (uint64_t i = _cmd_info.first_index_; i < index_buffer->count_; ++i) {
            start = std::min(start, *(static_cast<uint32_t*>(index_buffer->data_) + i));
            end = std::max(end, *(static_cast<uint32_t*>(index_buffer->data_) + i));
        }

        for (uint64_t i = start; i <= end; ++i) {
            alignas(16) auto v0 = Lamp::Vec4f(vertices[i].x, vertices[i].y, vertices[i].z, 1.0f);
            v0 = uniform->mvp * v0;
            ClipToNdc(v0);
            NdcToWindow(viewport_transform, v0);

            memcpy(&raster_data[i * sizeof(Lamp::Vec4f)], &v0, sizeof(Lamp::Vec4f));
        }

        auto& color_target = _cmd_info.render_info_->_color_att->image_;
        auto& depth_target = _cmd_info.render_info_->_depth_att->image_;

        auto new_vertices = reinterpret_cast<const Lamp::Vec4f*>(raster_data);
        const int x = static_cast<int>(view_port->x);
        const int y = static_cast<int>(view_port->y);
        const auto uiwidth = static_cast<uint32_t>(view_port->width);
        const auto uiheight = static_cast<uint32_t>(view_port->height);
        for (uint64_t i = 0; i < index_buffer->count_; i += 3) {
            auto i0 = *(static_cast<uint32_t*>(index_buffer->data_) + i);
            auto i1 = *(static_cast<uint32_t*>(index_buffer->data_) + i+1);
            auto i2 = *(static_cast<uint32_t*>(index_buffer->data_) + i+2);

            alignas(16) Lamp::Vec4f v0 = new_vertices[i0];
            alignas(16) Lamp::Vec4f v1 = new_vertices[i1];
            alignas(16) Lamp::Vec4f v2 = new_vertices[i2];

            AABB2i aabb;
            aabb.min = {
                static_cast<int>(std::min(std::min(v0.x, v1.x), v2.x)),
                static_cast<int>(std::min(std::min(v0.y, v1.y), v2.y))
            };
            aabb.max = {
                static_cast<int>(std::max(std::max(v0.x, v1.x), v2.x)) + 1,
                static_cast<int>(std::max(std::max(v0.y, v1.y), v2.y)) + 1
            };

            auto edge = [](const Lamp::Vec4f& _v0, const Lamp::Vec4f& _v1, const Lamp::Vec4f& _v2) {
                const Lamp::Vec2f a = {_v2.x - _v0.x, _v2.y - _v0.y}; //ab
                const Lamp::Vec2f b = {_v1.x - _v0.x, _v1.y - _v0.y};//cd

                // ad - bc.
                return a.x * b.y - a.y * b.x;
            };

            // Cross product == Area of parallelogram made with the area of triangle * 2.
            // Note that this edge function basically does pseudo-cross product.
            float area;
            area = abs(edge(v0, v1, v2));

            for (int j = aabb.min.y; j < aabb.max.y; ++j) {
                for (int k = aabb.min.x; k < aabb.max.x; ++k) {
                    Lamp::Vec4f p = {static_cast<float>(k), static_cast<float>(j), 0, 0};

                    const float w0 = edge(v0, v1, p) / area;
                    const float w1 = edge(v1, v2, p) / area;
                    const float w2 = edge(v2, v0, p) / area;

                    bool is_inside = w0 >= 0 && w1 >= 0 && w2 >= 0;

                    p.z = w0 * v0.z + w1 * v1.z + w2 * v2.z;

                    uint8_t r[] = {0, 0, 255};
                    uint8_t g[] = {0, 255, 0};
                    uint8_t b[] = {255, 0, 0};

                    if (is_inside) {
                        uint8_t color[] = {static_cast<uint8_t>(255 * w0),
                                            static_cast<uint8_t>(255 * w1),
                                            static_cast<uint8_t>(255 * w2)};

                        if (static_cast<int>(p.x) >= x
                            && static_cast<int>(p.x) < x + uiwidth
                            && static_cast<int>(p.y) >= y
                            && static_cast<int>(p.y) < y + uiheight) {


                            void* depth_ptr = static_cast<uint8_t *>(depth_target.Data())
                            + (depth_target.Width() * static_cast<uint32_t>(p.y) + static_cast<uint32_t>(p.x))
                            * depth_target.Stride();
                            uint16_t depth;
                            memcpy(&depth, depth_ptr, sizeof(uint16_t));

                            // Depth test.
                            // Potentially add depth compare op to pipeline.
                            if (depth < p.z) {
                                void* color_ptr = static_cast<uint8_t *>(color_target.Data())
                                    + (color_target.Width() * static_cast<uint32_t>(p.y) + static_cast<uint32_t>(p.x))
                                    * color_target.Stride();

                                depth = static_cast<uint16_t>(p.z);
                                memcpy(color_ptr, color, color_target.Stride());
                                memcpy(depth_ptr, &depth, depth_target.Stride());
                            }
                        }
                    }
                }
            }
        }


#endif
    }
}


inline void Render::Draw(const RenderCmdInfo& _cmd_info) {
    switch (_cmd_info.uniform_->sType) {
        case ShaderName::PointShader:
            DrawPointShader(_cmd_info);
            break;
        case ShaderName::LineShader:
            DrawTriangleLineShader(_cmd_info);
            break;
        case ShaderName::RasterShader:
            DrawRasterShader(_cmd_info);
            break;
        case ShaderName::Count:
            // Not implemented
            break;
        default: ;
    }
}