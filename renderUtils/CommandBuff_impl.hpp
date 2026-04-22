#include "CommandBuff.h"
#include "Render.h"
#include "RenderInfo.h"
#include "Viewport.h"

void CommandBuff::Execute() {
    RenderCmdInfo cmd_info;

    for (auto& exe : execution_list_) {
        switch (exe.type_) {
            case CmdType::SetViewport:
                cmd_info.view_port_ = static_cast<const Viewport*>(exe.data_);
                break;
            case CmdType::SetRenderInfo:
                cmd_info.render_info_ = static_cast<const RenderInfo*>(exe.data_);
                break;
            case CmdType::BindPipeline:
                cmd_info.pipeline_ = static_cast<const Pipeline*>(exe.data_);
                break;
            case CmdType::BindUniform:
                cmd_info.uniform_ = static_cast<const Render::ShaderFootprint*>(exe.data_);
                break;
            case CmdType::BindVertexBuffer:
                cmd_info.vertex_buffer_ = static_cast<const VertexBuffer*>(exe.data_);
                break;
            case CmdType::BindIndexBuffer:
                cmd_info.index_buffer_ = static_cast<const IndexBuffer*>(exe.data_);
                break;
            case CmdType::DrawIndexed:
                // Color
                // It should query number of attachmenets depending on the shader.
                cmd_info.first_index_ = (reinterpret_cast<uint64_t>(exe.data_));
                Render::Draw(cmd_info);
                // Depth
                break;
            default:
                // Handle error.
                break;
        }
    }
}
