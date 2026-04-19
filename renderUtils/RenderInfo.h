#ifndef TINYTINYRENDERER_RENDERINFO_H
#define TINYTINYRENDERER_RENDERINFO_H
#include "AttachmentInfo.h"

template<PixelFormat color_format = PixelFormat::Count,
         PixelFormat depth_format = PixelFormat::Count>
struct RenderInfo {
    uint8_t _color_count;
    AttInfo<color_format>* _color_att;
    AttInfo<depth_format>* _depth_att;
};

#endif //TINYTINYRENDERER_RENDERINFO_H