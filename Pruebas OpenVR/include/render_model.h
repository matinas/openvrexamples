#ifndef RENDER_MODEL_H
#define RENDER_MODEL_H

#define GLEW_STATIC
#include <GL\glew.h>

#include <iostream>
#include <SDL.h>
#include <SDL_opengl.h>
#include <openvr.h>

using namespace std;
using namespace vr;

class RenderModel {
public:
	RenderModel();
	RenderModel(const char*);
	~RenderModel();

	bool init();
	void update();
	void draw();
	void clean_up();

	GLuint get_texture_id();

private:
	GLuint render_model_vao, render_model_vbo, render_model_ebo;
	GLuint render_model_texture;
	GLsizei vertex_count;

	char* name;

	bool setup_buffers(const RenderModel_t&,const RenderModel_TextureMap_t&);
};

#endif