#ifndef TINYTINYRENDERER_VIEWPORT_H
#define TINYTINYRENDERER_VIEWPORT_H


struct Viewport {
    float    x; // left
    float    y; // upper corner
    float    width;
    float    height;
    float    near;
    float    far;
};

#endif //TINYTINYRENDERER_VIEWPORT_H