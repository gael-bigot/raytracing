# Raytracing

Coded this for a school project, so all the comments are in french sorry

Build with GCC:
```
make
```

Render .obj:
```
./render model.obj Xres Yres field_of_view max_reflexions iterations_per_pixels
```
Unfortunately the camera position and lighting angle are hard-coded but I can't bother fixing that for now
Should be a quick fix in render.c 

![donut.obj](https://github.com/gael-bigot/raytracing/imageFolder/donut.bmp)
