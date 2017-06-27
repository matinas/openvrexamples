#include "render_model.h"

RenderModel::RenderModel() {}

RenderModel::RenderModel(const char* rm_name) {

	this->name = new char[strlen(rm_name)];
	strcpy(this->name,rm_name);
}

RenderModel::~RenderModel() {

	clean_up();

	char* c_name = this->name;
	this->name = NULL;
	delete c_name;
}

GLuint RenderModel::get_texture_id() {
	return render_model_texture;
}

// Load render model data into RenderModel elements
bool RenderModel::init() {

	if (strlen(name) <= 0)
		return false;

	RenderModel_t *model;
	EVRRenderModelError error;
	while (1)
	{
		cout << "Starting loading render model's model (" << name << ")" << endl;
		error = VRRenderModels()->LoadRenderModel_Async(name, &model);
		if (error != VRRenderModelError_Loading)
			break;

		SDL_Delay(1);
	}
	cout << "Render model's model succesfully loaded" << endl;

	if (error != VRRenderModelError_None)
	{
		cout << "Unable to load render model " << name << ". Error code: " << VRRenderModels()->GetRenderModelErrorNameFromEnum(error);
		vr::VRRenderModels()->FreeRenderModel(model);
		return false;
	}

	RenderModel_TextureMap_t *rm_texture;
	while (1)
	{
		cout << "Starting loading render model's texture (" << name << ")" << endl;
		error = VRRenderModels()->LoadTexture_Async(model->diffuseTextureId, &rm_texture);
		if (error != vr::VRRenderModelError_Loading)
			break;

		SDL_Delay(1);
	}
	cout << "Render model's texture succesfully loaded" << endl;

	if (error != VRRenderModelError_None)
	{
		cout << "Unable to load render texture id " << model->diffuseTextureId << " for render model " << name;
		VRRenderModels()->FreeRenderModel(model);
		return false;
	}

	if (!setup_buffers(*model,*rm_texture))
		return false;

	VRRenderModels()->FreeRenderModel(model);
	VRRenderModels()->FreeTexture(rm_texture);

	return true;
}

// Fills render model's OpenGL buffers
bool RenderModel::setup_buffers(const RenderModel_t &rm_model, const RenderModel_TextureMap_t &rm_texture)
{
	vertex_count = rm_model.unTriangleCount * 3;

	cout << "Starting filling data buffers for render model..." << endl;

	// Create and bind a VAO to hold state for this model
	glGenVertexArrays(1, &render_model_vao);
	glBindVertexArray(render_model_vao);
	
	// Populate a vertex buffer
	glGenBuffers(1, &render_model_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, render_model_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(RenderModel_Vertex_t) * rm_model.unVertexCount, rm_model.rVertexData, GL_STATIC_DRAW);

	// Identify the components in the vertex buffer
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(RenderModel_Vertex_t), (void *) offsetof(RenderModel_Vertex_t, vPosition));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(RenderModel_Vertex_t), (void *) offsetof(RenderModel_Vertex_t, vNormal));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(RenderModel_Vertex_t), (void *) offsetof(RenderModel_Vertex_t, rfTextureCoord));

	cout << "Vertex buffer filled succesfully" << endl;

	// Populate an index buffer
	glGenBuffers(1,&render_model_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, render_model_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t) * vertex_count, rm_model.rIndexData, GL_STATIC_DRAW);

	cout << "Index buffer filled succesfully" << endl;

	glBindVertexArray(0);

	// Create a texture buffer
	glGenTextures(1,&render_model_texture);
	glBindTexture(GL_TEXTURE_2D,render_model_texture);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rm_texture.unWidth, rm_texture.unHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, rm_texture.rubTextureMapData);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	GLfloat fLargest;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &fLargest);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, fLargest);

	glBindTexture(GL_TEXTURE_2D, 0);

	cout << "Texture successfully registered" << endl;

	return true;
}

void RenderModel::update()
{

}

void RenderModel::draw()
{
	glBindVertexArray(render_model_vao);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, render_model_texture);

	glDrawElements(GL_TRIANGLES, vertex_count, GL_UNSIGNED_SHORT, 0);

	glBindVertexArray(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

void RenderModel::clean_up()
{
	glDeleteTextures(1,&render_model_texture);
	glDeleteBuffers(1,&render_model_vbo);
	glDeleteBuffers(1,&render_model_ebo);
	glDeleteVertexArrays(1,&render_model_vao);
	glDisableVertexAttribArray(0);
}