#ifndef TINYTINYRENDERER_COMMANDBUFF_H
#define TINYTINYRENDERER_COMMANDBUFF_H
#include "AttachmentInfo.h"
#include "Pipeline.h"
#include "special-lamp/lampList.h++"



enum class CmdType {
    Invalid,
    BeginRender,
    EndRender,
    SetViewport,
    SetRenderInfo,
    ColorAttCount,
    ColorAtt,
    DepthAtt,
    BindPipeline,
    BindUniform,
    BindVertexBuffer,
    VertexBufferBind,
    BindIndexBuffer,
    IndexBufferBind,
    DrawIndexed,
    Count
};

struct CmdBlock {
    CmdType type_ = CmdType::Invalid;
    const void* data_ = nullptr;
};

// Ultimately, command buffers life cycle should be :
// Initial->Recording->Executable->Pending->Invalidate->Initial.
class CommandBuff {
    friend class RenderCmd;

    template<PixelFormat color_format, PixelFormat depth_format>
    struct RenderCmdInfo {
        Lamp::Vector<AttInfo<color_format>> color_att_infos_ = {};
        AttInfo<depth_format> depth_att_info_ = {};
        WindingOrder front_face = WindingOrder::Count;
        Lamp::Vector<Image<color_format>> color_atts_;
        Image<depth_format> depth_att_;
        ShaderName shader_ = ShaderName::Count;
    };


    bool is_active_ = false;
    bool is_rendering_ = false;

    Lamp::list<CmdBlock> execution_list_;
public:
    bool IsExecutable() const {return !is_active_ && !is_rendering_;}
    void Execute() const;
};

#endif //TINYTINYRENDERER_COMMANDBUFF_H