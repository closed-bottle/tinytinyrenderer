#ifndef TINYTINYRENDERER_RENDERCOMMAND_H
#define TINYTINYRENDERER_RENDERCOMMAND_H

#include "CommandBuff.h"
#include "RenderBuffer.h"
#include "RenderInfo.h"
#include "Viewport.h"

enum class WindingOrder;
struct Pipeline;
struct VertexBuffer;

struct RenderCmdInfo {
    const Viewport* view_port_;
    const RenderInfo* render_info_;
    const Pipeline* pipeline_;
    const Render::ShaderFootprint* uniform_;
    const VertexBuffer* vertex_buffer_;
    const IndexBuffer* index_buffer_;
    const WindingOrder* front_face_;
    const ShaderName* shader_;
    uint64_t first_index_ = 0;
};

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
    static void SetRenderInfo(CommandBuff& _cmd, const RenderInfo& _render_info) {
        _cmd.execution_list_.push_back({CmdType::SetRenderInfo,
            reinterpret_cast<const void*>(&_render_info)});
    }


    static void BindPipeline(CommandBuff& _cmd, const Pipeline& _pipeline) {
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