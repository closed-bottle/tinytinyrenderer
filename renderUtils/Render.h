#ifndef TINYTINYRENDERER_RENDER_H
#define TINYTINYRENDERER_RENDER_H

#include "special-lamp/lampMath.h++"

struct RenderCmdInfo;

enum class ShaderName {
    PointShader,
    LineShader,
    RasterShader,
    Count
};


class Render {
public:

    struct ShaderFootprint {
        ShaderName sType = ShaderName::Count;

        explicit ShaderFootprint(const ShaderName& _sType) : sType(_sType) {}
        virtual ~ShaderFootprint() = default;
    };

    struct UMvp : ShaderFootprint {
        Lamp::Mat4f mvp;

        UMvp(const ShaderName& _sType, const Lamp::Mat4f& _mvp)
            : ShaderFootprint(_sType), mvp(_mvp) {}
    };

    static void Draw(const RenderCmdInfo& _cmd_info);
};

#include "Render_impl.hpp"
#endif //TINYTINYRENDERER_RENDER_H