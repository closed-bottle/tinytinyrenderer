# SwRenderer
 Software renderer for hobbyist, with minimal features.

It is under construction for now, I'm revisiting it after 7 years.

Current build only supports simple line drawing without triangle filling.

> Inspired by tinyrenderer by Dmitry V. Sokolov(ssloy)
> 
> https://github.com/ssloy/tinyrenderer

## Features
 - No graphics API.
 - CPU rasterization.
 - OBJ loading.
 - TGA output writing.

![Alt text](suzanne_wireframe.png "Images")

### How to build

### Know issues
 
### Measurement
  Note that those benchmarks are not formally taken, and only used to briefly show
 how my older implementation is unoptimized.
  It includes I/O, and output image is 3000 * 2500 with 3 channels.
 - Old(current) build before refactoring takes 6.163932 on average(5 samples)

### Credits
 The function "plotLine" adapted from :
 > "A Rasterizing Algorithm for Drawing Curves"
 > by Alois Zingl
 > 
 Licensed under the MIT License.
