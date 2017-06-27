#include "shader.h"

Shader::Shader() {}

Shader::Shader(char* vertex_shader, char* fragment_shader, char* geometry_shader) {

	vertex_shader_path = new char[strlen(vertex_shader)];
	strcpy(vertex_shader_path,vertex_shader);
	fragment_shader_path = new char[strlen(fragment_shader)];
	strcpy(fragment_shader_path,fragment_shader);
	geometry_shader_path = new char[strlen(geometry_shader)];
	strcpy(geometry_shader_path,geometry_shader);
}

Shader::~Shader() {

	clean_up();

	delete vertex_shader_path;
	delete fragment_shader_path;
	delete geometry_shader_path;
}

GLuint Shader::get_shader() {

	return shader_program;
}

bool Shader::read_shader(char* filePath, string* str_shader) {

	ifstream f(filePath);

	if (!f.good()) {
		cout << "Cannot open file " << filePath << endl;
		return false;
	}

	stringstream buffer;
	buffer << f.rdbuf();

	*str_shader = buffer.str();

	return true;
}

bool Shader::init() {

	int compiled = 0;

	if (vertex_shader_path != NULL && (strcmp(vertex_shader_path,"") == 0) && (fragment_shader_path != NULL && (strcmp(fragment_shader_path,"") == 0)))
	{
		cout << "There's no shader to init" << endl;
		return false;
	}
	else
	{
		shader_program = glCreateProgram();

		// Bind attribute index 0 (coordinates) to in_Position and attribute index 1 (color) to in_Color
		// Attribute locations must be setup before calling glLinkProgram
		// NOTE: this step is not strictly necessary as we have already binded the attributes with glVertexAttribPointer
//		glBindAttribLocation(shader_program, 0, "in_Position");
//		glBindAttribLocation(shader_program, 1, "in_Color");
//		glBindAttribLocation(shader_program, 2, "in_Texcoord");

		cout << "Shader program successfully created and attributes already binded" << endl;
	}

	if (strcmp(vertex_shader_path,"") != 0) {

		// Create a new vertex shader
		vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);

		// Send the created shader to OpenGL
		string str_vertex_shader;
		if (read_shader(vertex_shader_path,&str_vertex_shader)) {
			GLint vs_len = str_vertex_shader.length();
			const char* vs_data = str_vertex_shader.data();
			glShaderSource(vertex_shader_id,1,&vs_data,&vs_len);

			// Compile the vertex shader
			glCompileShader(vertex_shader_id);
	
			// Ask OpenGL if the shaders was compiled
			glGetShaderiv(vertex_shader_id, GL_COMPILE_STATUS, &compiled);
				if (compiled == GL_FALSE) {
				print_shader_compilation_info(vertex_shader_id);
				cout << "There was a problem trying to compile the vertex shader" << endl;
				return false;
			}

			// If all was fine with the creation and compilation, attach the vertex shader to the already created shader program
			glAttachShader(shader_program,vertex_shader_id);

			cout << "Vertex shader correctly generated and attached to shader program" << endl;
		}
		else
			return false;
	}
	else
		cout << "There's no vertex shader to load" << endl;

	if (strcmp(fragment_shader_path,"") != 0) {

		fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);

		string str_fragment_shader;
		if (read_shader(fragment_shader_path,&str_fragment_shader)) {
			GLint fs_len = str_fragment_shader.length();
			const char* fs_data = str_fragment_shader.data();
			glShaderSource(fragment_shader_id,1,&fs_data,&fs_len);
			glCompileShader(fragment_shader_id);

			glGetShaderiv(fragment_shader_id, GL_COMPILE_STATUS, &compiled);
			if (compiled == GL_FALSE) {
				print_shader_compilation_info(fragment_shader_id);
				cout << "There was a problem trying to compile the fragment shader" << endl;
				return false;
			}

			glAttachShader(shader_program,fragment_shader_id);

			// Since a fragment shader is allowed to write to multiple buffers, you need to explicitly specify which output is written to which buffer
			// However, since this is 0 by default and there's only one output right now, the following line of code is not really necessary
			glBindFragDataLocation(shader_program,0,"fragColor");

			cout << "Fragment shader correctly generated and attached to shader program" << endl;
		}
		else
			return false;
	}
	else
		cout << "There's no fragment shader to load" << endl;

	if (strcmp(geometry_shader_path,"") != 0) {

		geometry_shader_id = glCreateShader(GL_GEOMETRY_SHADER);

		string str_geometry_shader;
		if (read_shader(geometry_shader_path,&str_geometry_shader)) {
			GLint gs_len = str_geometry_shader.length();
			const char* gs_data = str_geometry_shader.data();
			glShaderSource(geometry_shader_id,1,&gs_data,&gs_len);
			glCompileShader(geometry_shader_id);

			glGetShaderiv(geometry_shader_id, GL_COMPILE_STATUS, &compiled);
			if (compiled == GL_FALSE) {
				print_shader_compilation_info(geometry_shader_id);	// It's useful to print this information even when there's no error as it may contain some warnings during compilation
				cout << "There was a problem trying to compile the geometry shader" << endl;
				return false;
			}

			glAttachShader(shader_program,geometry_shader_id);

			cout << "Geometry shader correctly generated and attached to shader program" << endl;
		}
		else
			return false;
	}
	else
		cout << "There's no geometry shader to load" << endl;

	glLinkProgram(shader_program);

	glGetProgramiv(shader_program, GL_LINK_STATUS, &compiled);
	if (compiled == GL_FALSE) {
		print_program_linking_info(shader_program);
		cout << "There was a problem trying to assemble the shader program" << endl;
		return false;
	}

	return true;
}

bool Shader::use_program() {

	glUseProgram(shader_program);

	return true;
}

bool Shader::set_mvp_matrix(const glm::mat4 mvp) {

	int matrixID = glGetUniformLocation(shader_program,"mvp");
	glUniformMatrix4fv(matrixID,1,GL_FALSE,glm::value_ptr(mvp));

	return true;
}

bool Shader::set_model_matrix(const glm::mat4 model) {

	int matrixID = glGetUniformLocation(shader_program,"model");
	glUniformMatrix4fv(matrixID,1,GL_FALSE,glm::value_ptr(model));

	return true;
}

bool Shader::set_view_matrix(const glm::mat4 view) {

	int matrixID = glGetUniformLocation(shader_program,"view");
	glUniformMatrix4fv(matrixID,1,GL_FALSE,glm::value_ptr(view));

	return true;
}

bool Shader::set_projection_matrix(const glm::mat4 projection) {

	int matrixID = glGetUniformLocation(shader_program,"projection");
	glUniformMatrix4fv(matrixID,1,GL_FALSE,glm::value_ptr(projection));

	return true;
}

bool Shader::clean_up() {

	glUseProgram(0);

	glDetachShader(shader_program, vertex_shader_id);
	glDetachShader(shader_program, fragment_shader_id);
	glDetachShader(shader_program, geometry_shader_id);

	glDeleteShader(vertex_shader_id);
	glDeleteShader(fragment_shader_id);
	glDeleteShader(geometry_shader_id);

	glDeleteProgram(shader_program);

	return true;
}

void Shader::print_shader_compilation_info(int32_t shaderId)
{
    // Find length of shader info log
    int maxLength;
    glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

    // Get shader info log
    char* shaderInfoLog = new char[maxLength];
    glGetShaderInfoLog(shaderId, maxLength, &maxLength, shaderInfoLog);

    string log = shaderInfoLog;
 
    if (log.length())
    {
        cout << "=======================================" << endl;
        cout <<  shaderInfoLog << endl;
        cout << "=======================================" << endl << endl;
    }

    // Print shader info log
    delete shaderInfoLog;
}

void Shader::print_program_linking_info(int32_t shaderId)
{
    // Find length of shader linking log
    int maxLength;
	glGetProgramiv(shaderId, GL_INFO_LOG_LENGTH, &maxLength);

    // Get shader info log
    char* shaderProgramInfoLog = new char[maxLength];
    glGetProgramInfoLog(shaderId, maxLength, &maxLength, shaderProgramInfoLog );

    cout << "Linker error message : " << shaderProgramInfoLog << endl;

    // Print shader info log
    delete shaderProgramInfoLog;
}