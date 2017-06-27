#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#define GLEW_STATIC

#include "shader.h"
#include "models.h"
#include "render_model.h"

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define GLEW_STATIC
#include <GL\glew.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <GL\GLU.h>
#include <SOIL.h>

#include <openvr.h>
#include <glm.hpp>

using namespace std;
using namespace vr;

class SceneManager {
public:
	~SceneManager();

	static SceneManager* getInstance();

	int getFin() { return app_end; }

	int init();
	void update();
	void draw();
	void exit();

private:
	SceneManager();

	static SceneManager* instance;

	Shader *scene_shader, *screen_shader, *render_model_shader;

	IVRSystem* vr_context;
	IVRRenderModels* vr_render_models;
	IVRCompositor* vr_compositor;

	SDL_Window* companion_window;
	SDL_GLContext gl_context;

	// Used to define the transformation for the square
	glm::mat4 model;

	bool app_end, v_blank;
	int w_width, w_height, w_pos_x, w_pos_y;
	uint32_t render_width, render_height;
	float time_1;

	string m_strDriver;
	string m_strDisplay;
	float fNearZ, fFarZ;
	int base_stations_count;

	RenderModel* render_models[k_unMaxTrackedDeviceCount];
	TrackedDevicePose_t tracked_device_pose[k_unMaxTrackedDeviceCount];

	glm::mat4 tracked_device_pose_matrix[k_unMaxTrackedDeviceCount];	// Used to maintain the model transformation for each tracking device
	glm::mat4 hmd_pose_matrix;											// Used to maintain the HMD model transformation (obtained from tracked_device_pose_matrix)
	glm::mat4 projection_matrix_left, projection_matrix_right;			// Used to maintain the HMD projection matrix for each eye
	glm::mat4 view_matrix_left, view_matrix_right;						// Used to maintain the HMD view matrix for each eye (view matrix involves head position and eye-to-head position)

	// Buffer ids for the main 3D cube
	GLuint cube_texture;
	GLuint cube_texture_vbo, cube_texture_vao;

	// Buffer ids for the 2D square used to copy the frame buffer
	GLuint square_texture_vbo, square_texture_vao, square_texture_ebo;
	
	// Buffer ids for the left and rigth eyes frame buffer and stuff
	GLuint l_frame_buffer, l_tex_color_buffer, l_render_buffer_depth_stencil;
	GLuint r_frame_buffer, r_tex_color_buffer, r_render_buffer_depth_stencil;

	int initSDL();
	int initOpenGL();
	int initOpenVR();

	void process_vr_event(const vr::VREvent_t & event);

	bool setup_scene_buffers();
	bool setup_frame_buffer(GLsizei width,GLsizei height,GLuint &frame_buffer,GLuint &render_buffer_depth_stencil,GLuint &tex_color_buffer);
	bool setup_render_model(vr::TrackedDeviceIndex_t);

	glm::mat4 convert_SteamVRMat_to_GLMMat(const HmdMatrix34_t &matPose);
	glm::mat4 get_projection_matrix(const Hmd_Eye);
	glm::mat4 get_view_matrix(const Hmd_Eye);
};

#endif