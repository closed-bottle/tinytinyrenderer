#ifndef TINYTINYRENDERER_RENDERCOMMAND_H
#define TINYTINYRENDERER_RENDERCOMMAND_H
#include "AttachmentInfo.h"
#include "CommandBuff.h"
#include "RenderBuffer.h"
#include "Viewport.h"

struct VertexBuffer;

struct RenderCmd {
    static void BeginCmd(CommandBuff& _cmd) {
        _cmd.is_active_ = true;
    }
    static void EndCmd(CommandBuff& _cmd) {
        _cmd.is_active_ = false;
    }
    static void BeginRender(CommandBuff& _cmd) {
        _cmd.is_rendering_ = true;
    }
    static void EndRender(CommandBuff& _cmd) {
        _cmd.is_rendering_ = false;
    }

    static void SetViewport(CommandBuff& _cmd, const Viewport& _viewport) {
        _cmd.map_["viewport"] = reinterpret_cast<const void*>(&_viewport);
    }

    template<PixelFormat color_format = PixelFormat::Count, PixelFormat depth_format = PixelFormat::Count>
    static void SetRenderInfo(CommandBuff& _cmd, uint8_t _color_count, AttInfo<color_format>* _color_att,
                                AttInfo<depth_format>* _depth_att) {
        Lamp::String c = "color_att000";

        for (uint8_t i = 0; i < _color_count; ++i) {
            _cmd.map_[c] = reinterpret_cast<const void*>(&_color_att[i]);
            c[c.length() - 2]++; // only 10 color attachments are supported at this moment for simplicity.
        }

        _cmd.map_["depth_att"] = reinterpret_cast<const void*>(&_depth_att);
    }

    template<PixelFormat colorformat, PixelFormat depthFormat>
    static void BindPipeline(CommandBuff& _cmd, const Pipeline<colorformat, depthFormat>& _pipeline) {
        _cmd.map_["pipeline"] = reinterpret_cast<const void*>(&_pipeline);
    }

    static void BindUniform(CommandBuff& _cmd, const Render::ShaderFootprint& _uniform) {
        _cmd.map_["uniform"] = reinterpret_cast<const void*>(&_uniform);
    }

    static void BindVertexBuffer(CommandBuff& _cmd, const VertexBuffer& _buffer, const uint32_t _bind) {
        Lamp::String num = "0";

        // TODO : Better implementation. For example, String.back() and itos().
        num = num + Lamp::String("0");
        num[num.length() - 2] = (_bind & 0xFF000000) >> 24;
        num = num + Lamp::String("0");
        num[num.length() - 2] = (_bind & 0xFF000000) >> 16;
        num = num + Lamp::String("0");
        num[num.length() - 2] = (_bind & 0xFF000000) >> 8;
        num = num + Lamp::String("0");
        num[num.length() - 2] = (_bind & 0xFF000000) >> 0;

        Lamp::String vertex_bind = "vertex_bind";
        Lamp::String vertex_count = "vertex_count";

        _cmd.map_[vertex_bind + num] = reinterpret_cast<const void*>(_buffer.data_);
        _cmd.map_[vertex_count + num] = reinterpret_cast<const void*>(_buffer.count_);
    }


    static void BindIndexBuffer(CommandBuff& _cmd, const IndexBuffer& _buffer, const uint32_t _bind) {
        Lamp::String num = "0";

        // TODO : Better implementation. For example, String.back() and itos().
        num = num + Lamp::String("0");
        num[num.length() - 2] = (_bind & 0xFF000000) >> 24;
        num = num + Lamp::String("0");
        num[num.length() - 2] = (_bind & 0xFF000000) >> 16;
        num = num + Lamp::String("0");
        num[num.length() - 2] = (_bind & 0xFF000000) >> 8;
        num = num + Lamp::String("0");
        num[num.length() - 2] = (_bind & 0xFF000000) >> 0;

        Lamp::String index_bind = "index_bind";
        Lamp::String index_count = "index_count";

        _cmd.map_[index_bind + num] = reinterpret_cast<const void*>(_buffer.data_);
        _cmd.map_[index_count + num] = reinterpret_cast<const void*>(_buffer.count_);
    }

    // No instancing is implemented yet.
    static void DrawIndexed(CommandBuff& _cmd, const uint64_t _first_index) {
        _cmd.map_["draw_indexed"] = reinterpret_cast<const void *>(_first_index);
    }
};

#endif //TINYTINYRENDERER_RENDERCOMMAND_H