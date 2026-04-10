# tinytinyrenderer
Tiny software renderer for hobbyst, with minimal features.

It is under construction for now, I'm revisiting it after 7 years.

Current build is broken, needs full refactoring. 

## Features
 - No graphics API.
 - CPU rasterization.
 - OBJ loading.
 - TGA output writing.

![Alt text](suzanne_wireframe.png "Images")



### Know issuees
 - Depth buffer is not implemented correctly or not implemented at all.
 - Failed writing to file in Release build
 - pragma pack.

 
### Measurement
  Note that those benchmarks are not formally taken, and only used to briefly show
 how my older implementation is unoptimized.
  It includes I/O, and output image is 3000 * 2500 with 3 channels.
 - Old(current) build before refactoring takes 6.163932 on average(5 samples)
