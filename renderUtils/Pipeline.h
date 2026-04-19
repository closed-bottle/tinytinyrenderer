#ifndef TINYTINYRENDERER_PIPELINE_H
#define TINYTINYRENDERER_PIPELINE_H
#include "Image.h"
#include "Render.h"


enum class WindingOrder {
    CCW,
    CW,
    Count
};

template<PixelFormat colorformat, PixelFormat depthFormat>
struct Pipeline {
    WindingOrder front_face_;

    uint8_t color_attachment_count_ = 0;
    Image<colorformat>* color_render_target_;

    uint8_t depth_attachment_count_ = 0;
    Image<depthFormat>* depth_render_target_;

    ShaderName shader_;
};




#endif //TINYTINYRENDERER_PIPELINE_H