#ifndef SHADER_MANAGER_H
#define SHADER_MANAGER_H

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include <mat4x4.hpp>
#include <gtc\type_ptr.hpp>
#include <GL\glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <GL\GLU.h>

using namespace std;

class Shader {
public:
	Shader();
	Shader(char*,char*,char*);
	~Shader();

	GLuint get_shader();

	bool init();
	bool read_shader(char*,string*);
	bool use_program();
	bool set_mvp_matrix(const glm::mat4);
	bool set_model_matrix(const glm::mat4);
	bool set_view_matrix(const glm::mat4);
	bool set_projection_matrix(const glm::mat4);
	bool clean_up();

private:
	
	char *vertex_shader_path, *fragment_shader_path, *geometry_shader_path;

	GLuint vertex_shader_id, fragment_shader_id, geometry_shader_id;
	GLuint shader_program;
	
	void print_shader_compilation_info(int32_t shaderId);
	void print_program_linking_info(int32_t shaderId);
};

#endif