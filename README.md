SDL2 3D
=======
![basic demo](demo.gif)


This is a fully functional PS1-style 3D renderer written in software (kinda) using SDL_RenderGeometry.
It uses cglm to perform the necessary linear algebra as well as assist in the frustum culling.

Features:

 - Draw arbitrary triangle lists
 - Affine texture mapping
 - Frustum culling (only by vertex, doesn't work well with giant triangles)
 - Depth sorting (painter's algorithm, no depth buffer)