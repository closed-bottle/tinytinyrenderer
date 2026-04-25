# SwRenderer
 Software renderer for hobbyist, with minimal features.

It is under construction for now, I'm revisiting it after 7 years.

Current build only supports simple line drawing without triangle filling.

> Inspired by tinyrenderer by Dmitry V. Sokolov(ssloy)
> 
> https://github.com/ssloy/tinyrenderer

## Features
 - No graphics API.
 - Vulkan-like interface.
 - CPU rasterization.
 - OBJ loading.
 - TGA output writing.

![Alt text](media/2cc524ed.png "Images")

### How to build

### Know issues
 - x64 only.
### Measurement

 Before/After noted below means before any optimization/algorithm implementation and after.
 Any implementation applied on "after" build will be noted next to the scene description.

  - Scene #1 rendered in 3000*2500, 3 channels.
  - Scene #2 rendered in 8192*8192, 3 channels.
    - Attached scene is scale downed to 2048*2048.

Render | ![Alt text](media/suzanne_wireframe.png "Images") | ![Alt text](media/2cc524ed.png "Images")          |
--- |---------------------------------------------------|---------------------------------------------------|
Before | 6.163932s                                         | 0.7941572s                                        |
After | 0.05273832s(More than 100x faster)                | 0.7359066s(7%, without SIMD. 0.7408656s for SIMD) |
### Credits
 The function "plotLine" adapted from :
 > "A Rasterizing Algorithm for Drawing Curves"
 > by Alois Zingl
 > 
 Licensed under the MIT License.
