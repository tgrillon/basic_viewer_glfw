#pragma once

/*************OPENGL WINDOW PARAMS*************/

#define WINDOW_WIDTH_INIT 500
#define WINDOW_HEIGHT_INIT 450

#define WINDOW_SAMPLES 4

/*********************************************/

#define CLIPPING_PLANE_RENDERING_TRANSPARENCY 0.5f

#define SIZE_POINTS 7.0f
#define SIZE_EDGES  3.1f
#define SIZE_RAYS   3.1f
#define SIZE_LINES  3.1f

#define FACES_MONO_COLOR    {60, 60, 200}
#define VERTICES_MONO_COLOR {200, 60, 60}
#define EDGES_MONO_COLOR    {0, 0, 0}
#define RAYS_MONO_COLOR     {0, 0, 0}
#define LINES_MONO_COLOR    {0, 0, 0}

#define LIGHT_POSITION {0.0f, 0.0f, 0.0f, 0.0f}

#define AMBIENT_COLOR  {0.6f, 0.5f, 0.5f, 1.f}
#define DIFFUSE_COLOR  {0.9f, 0.9f, 0.9f, 1.0f}
#define SPECULAR_COLOR {0.0f, 0.0f, 0.0f, 1.0f}

#define SHININESS 0.5f

#define CAM_MOVE_SPEED 0.4f 
#define CAM_ROT_SPEED 0.05f 
