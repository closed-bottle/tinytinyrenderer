#ifndef TINYTINYRENDERER_ATTACHMENTINFO_H
#define TINYTINYRENDERER_ATTACHMENTINFO_H
#include "Image.h"


enum class LoadOp {
    LOAD_OP_CLEAR // default
};

enum class StoreOp {
    STORE_OP_STORE, // default
    STORE_OP_DONT_CARE
};

template<PixelFormat PF>
struct AttInfo {
    Image<PF> image_;
    LoadOp load_op_;
    StoreOp store_op_;
    Texel<PF> clear_val_;
};

#endif //TINYTINYRENDERER_ATTACHMENTINFO_H