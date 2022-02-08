![](doc/splash.gif)

`terrain3` procedurally generates terrain and represents it with varying levels
of detail using a geometry clipmap. The terrain itself is created using
fractional Brownian motion where the accumulated derivative of the noise values
is used to scale the accumulated values. The analytical derivative of the
terrain is found, using which the normal of the terrrain is calculated with
little to no additional overhead. The terrain height and gradient values are
calculated using an OpenGL compute shader.

## Controls

* Hit `ENTER` for a demo.
* Use `F1`, `F2`, `F3`, `F4` to toggle wireframe, debug drawing, terrain
  normals, and debug information.
* Use the middle mouse button to rotate, use shift and the middle mouse button
  to pan, and use the scroll wheel to zoom.

## Performance

At its most detailed level, the terrain is represented with a resolution of
ten centimeters. The above image displays movement at about 200 meters per
second, which a system with a Ryzen 5 2600 and an RTX 2070 takes roughly
`0.013-0.017` ms to update the terrain and `0.14-0.18` ms to render it at 1080p.

## System requirements and dependencies

Windows 10. Building is tested with MSVC, but other compilers should work as
well. The following third-party dependencies are included:

* [glad2](https://gen.glad.sh/) for loading OpenGL and WGL.
* [imgui](https://github.com/ocornut/imgui) for rendering text.
* [stb](https://github.com/nothings/stb) for reading and writing PNG files.
* [Poly Haven](https://polyhaven.com/) for grass and cliff textures.

## Resources

* _Terrain Rendering Using GPU-Based Geometry Clipmaps_  
  Asirvatham, A. and Hoppe, H. [[link]](https://hhoppe.com/gpugcm.pdf)  
  The technique presented in this paper is used to represent the terrain at
  multiple resolutions.
* _Geometry Clipmaps: Simple Terrain Rendering with Level of Detail_  
  Savage, M. [[link]](https://mikejsavage.co.uk/blog/geometry-clipmaps.html)  
  Article on the above paper with helpful practical information.
* _OpenGL ES SDK for Android Tutorial_  
  Arm
  Software [[link]](https://arm-software.github.io/opengl-es-sdk-for-android/terrain.html#terrainIntroduction)  
  The code of this repository forms the base of this project.
* _Value Noise Derivatives_  
  Quillez,
  I. [[link]](https://iquilezles.org/www/articles/morenoise/morenoise.htm)  
  This article explains how derivatives can be extracted from value noise and
  how these derivatives can be used to make interesting terrain.
