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

![Alt text](suzanne_wireframe.png "Images")

### How to build

### Know issues
 - x64 only.
### Measurement
  Note that those benchmarks are not formally taken, and only used to briefly show
 how my older implementation is unoptimized.
  It includes I/O, and output image is 3000 * 2500 with 3 channels.
 - Old build before refactoring takes 6.163932 seconds on average(5 samples)
 - Current build with efficient line plotting algorithm takes 0.05273832 seconds on average(5 samples)
### Credits
 The function "plotLine" adapted from :
 > "A Rasterizing Algorithm for Drawing Curves"
 > by Alois Zingl
 > 
 Licensed under the MIT License.
