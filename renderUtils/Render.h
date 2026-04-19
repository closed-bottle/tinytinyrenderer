#ifndef TINYTINYRENDERER_RENDER_H
#define TINYTINYRENDERER_RENDER_H

#include "Image.h"
#include "Geometry.h"
#include "Mesh.h"
#include "RenderBuffer.h"
#include "special-lamp/lampMath.h++"

enum class ShaderName {
    PointShader,
    LineShader,
    Count
};


class Render {
public:

    struct ShaderFootprint {
        ShaderName sType = ShaderName::Count;

        ShaderFootprint(const ShaderName& _sType) : sType(_sType) {}
        virtual ~ShaderFootprint() {};
    };

    struct UMvp : ShaderFootprint {
        Lamp::Mat4f mvp;

        UMvp(const ShaderName& _sType, const Lamp::Mat4f& _mvp)
            : ShaderFootprint(_sType), mvp(_mvp) {}
    };

    template<PixelFormat PF>
    static void Draw(Image<PF>& _render_target,
                const VertexBuffer& _vb, const IndexBuffer& _ib,
                const ShaderFootprint* _uniform);
};

#include "Render_impl.hpp"
#endif //TINYTINYRENDERER_RENDER_H