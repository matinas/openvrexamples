#ifndef MODELS_H
#define MODELS_H

#include <SDL_opengl.h>

using namespace std;

const GLuint elements[] = {
    0, 1, 2,
    2, 3, 0
};

// As this texture will not be loaded with SOIL, the pixels in the texture will be addressed using OpenGL texture coordinates during drawing operations.
// These coordinates range from 0.0 to 1.0 where (0,0) is conventionally the bottom-left corner and (1,1) is the top-right corner of the texture image

const uint32_t vertices = 4;
const uint32_t floatsPerVertex = 9;
const GLfloat square_tex[vertices][floatsPerVertex] =
{
	// Position		     // Color				 // Textcoords
    { -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f },	// Top left
	{  1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f },	// Top right
    {  1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f },	// Bottom right 
    { -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f }	// Bottom left
};

// The loadTexture SOIL function begins loading the image at coordinate (0,0), so pay attention to this.
// As mentioned before, OpenGL expects the first pixel to be located in the bottom-left corner, which means that textures will be flipped when loaded with SOIL directly.
// To counteract that, we can take several approaches:
// 1. Use flipped Y coordinates for texture coordinates from now on, That means that (0,0) will be assumed to be the top-left corner instead of the bottom-left.
//	  This practice might make texture coordinates more intuitive as a side-effect. This was the approach applied below
// 2. Invert the Y axis as part of the camera definition, i.e: (0,-1,0) for the Up vector of the camera, and define all the vertices considering this transformation

const GLfloat cube_tex[] =	// Enable this for drawing the cube as GL_TRIANGLES (must enable also drawArray(GL_TRIANGLES) in draw() function)
{
	// Position		     // Color				// Textcoords
	-0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 1.0f,		// Front face (z = -0.5f)
     0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 1.0f,

    -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 1.0f,		// Back face (z = 0.5f)
     0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 0.0f,		// Left face (x = -0.5f)
    -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 0.0f,

     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 0.0f,		// Right face (x = 0.5f)
     0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 0.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 0.0f,

    -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 1.0f,		// Top face (y = 0.5f)
     0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0, 0.0f, 1.0f,

	-1.0f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0f, 0.0f,		// Planar base
     1.0f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0, 1.0f, 0.0f,
     1.0f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0, 1.0f, 1.0f,
     1.0f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0, 1.0f, 1.0f,
    -1.0f, -0.5f,  1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0f, 1.0f,
    -1.0f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0, 0.0f, 0.0f
};

#endif