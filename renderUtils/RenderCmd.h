#ifndef TINYTINYRENDERER_RENDERCOMMAND_H
#define TINYTINYRENDERER_RENDERCOMMAND_H
#include "AttachmentInfo.h"
#include "CommandBuff.h"
#include "RenderBuffer.h"
#include "RenderInfo.h"
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
        _cmd.execution_list_.push_back({CmdType::SetViewport,
                    reinterpret_cast<const void*>(&_viewport)});
    }

    template<PixelFormat color_format = PixelFormat::Count, PixelFormat depth_format = PixelFormat::Count>
    static void SetRenderInfo(CommandBuff& _cmd, RenderInfo<color_format, depth_format> _render_info) {
        _cmd.execution_list_.push_back({CmdType::SetRenderInfo,
            reinterpret_cast<const void*>(&_render_info)});
    }

    template<PixelFormat colorformat, PixelFormat depthFormat>
    static void BindPipeline(CommandBuff& _cmd, const Pipeline<colorformat, depthFormat>& _pipeline) {
        _cmd.execution_list_.push_back({CmdType::BindPipeline,
            reinterpret_cast<const void*>(&_pipeline)});
    }

    static void BindUniform(CommandBuff& _cmd, size_t _size, const Render::ShaderFootprint& _uniform) {
        _cmd.execution_list_.push_back({CmdType::BindUniform,
            reinterpret_cast<const void*>(&_uniform)});
    }

    static void BindVertexBuffer(CommandBuff& _cmd, const VertexBuffer& _buffer, const uint64_t _bind) {
        _cmd.execution_list_.push_back({CmdType::BindVertexBuffer,
            reinterpret_cast<const void*>(&_buffer)});
        _cmd.execution_list_.push_back({CmdType::VertexBufferBind,
            reinterpret_cast<const void*>(_bind)});
    }

    static void BindIndexBuffer(CommandBuff& _cmd, const IndexBuffer& _buffer, const uint32_t _bind) {
        _cmd.execution_list_.push_back({CmdType::BindIndexBuffer,
    reinterpret_cast<const void*>(&_buffer)});
        _cmd.execution_list_.push_back({CmdType::IndexBufferBind,
            reinterpret_cast<const void*>(_bind)});
    }

    // No instancing is implemented yet.
    static void DrawIndexed(CommandBuff& _cmd, const uint64_t _first_index) {
        _cmd.execution_list_.push_back({CmdType::DrawIndexed,
    reinterpret_cast<const void*>(_first_index)});
    }
};

#endif //TINYTINYRENDERER_RENDERCOMMAND_H