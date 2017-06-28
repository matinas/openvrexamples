// TODOs: 1. Solve the render models texture problem
//		  2. Use the resolve frame buffer used for implementing a kind of multipass antialiasing and check visual improvement: https://www.khronos.org/opengl/wiki/Multisampling
//		  3. Add some cool effects to the cube (e.g.: mirroring using stencil buffer from "Pruebas OpenGL3" project)
//		  4. Add a big cube (or textured sphere) including the whole scene in order to implement a simple environment
//		  5. Add some lighting to the project (implement lighting equations in shaders)

#include "scene_manager.h"

#include <string>
#include <gtc/matrix_transform.hpp> // Needed for importing GLM Extensions (including matrix transformations, etc)
									// To solve the issue reggarding the error LNK2001: símbolo externo "public: __thiscall glm::tvec4<float,0>::tvec4<float,0>(struct glm::tvec4<float,0> const &)" (??0?$tvec4@M$0A@@glm@@QAE@ABU01@@Z) sin resolver
									// The solution is here (adding a little code to setup.hpp): https://github.com/g-truc/glm/issues/377

SceneManager* SceneManager::instance = NULL;

SceneManager::SceneManager() {}

SceneManager::~SceneManager() {}

SceneManager* SceneManager::getInstance() {

	if (instance == NULL)
		instance = new SceneManager();

	return instance;
}

int SceneManager::init() {

	vr_context = NULL;
	vr_render_models = NULL;
	vr_compositor = NULL;

	app_end = false;
	v_blank = false;
	w_width = 1200; w_height = 900;
	w_pos_x = 100; w_pos_y = 100;

	base_stations_count = 0;

	time_1 = (float) SDL_GetTicks();

	// Calculate model-view-projection matrix

	model = glm::translate(glm::mat4(1.0f),glm::vec3(0.f,0.f,-3.f));

	view_matrix_left = view_matrix_right = glm::mat4(1.0);
	projection_matrix_left = projection_matrix_right = glm::mat4(1.0);

	for (int nDevice=0; nDevice<k_unMaxTrackedDeviceCount; nDevice++)
	{
		tracked_device_pose_matrix[nDevice] = glm::mat4(1.0);
		render_models[nDevice] = NULL;
	}

	scene_shader = new Shader(".\\.\\src\\scene_shader.vert",".\\.\\src\\scene_shader.frag","");
	screen_shader = new Shader(".\\.\\src\\screen_shader.vert",".\\.\\src\\screen_shader.frag","");
	render_model_shader = new Shader(".\\.\\src\\render_model_shader.vert",".\\.\\src\\render_model_shader.frag","");

	m_strDriver = "No Driver";
	m_strDisplay = "No Display";

	if ((initSDL() == -1) || (initOpenGL() == -1) || (initOpenVR() == -1))
		return -1;

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device property and turn it
//			into a std::string
//-----------------------------------------------------------------------------
string GetTrackedDeviceString(vr::IVRSystem *pHmd, vr::TrackedDeviceIndex_t unDevice, vr::TrackedDeviceProperty prop, vr::TrackedPropertyError *peError = NULL)
{
	uint32_t requiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice, prop, NULL, 0, peError);
	if( requiredBufferLen == 0 )
		return "";

	char *pchBuffer = new char[requiredBufferLen];
	requiredBufferLen = pHmd->GetStringTrackedDeviceProperty(unDevice,prop,pchBuffer,requiredBufferLen,peError);
	string sResult = pchBuffer;
	delete[] pchBuffer;
	return sResult;
}

//-----------------------------------------------------------------------------
// Purpose: Helper to get a string from a tracked device type class
//-----------------------------------------------------------------------------
string GetTrackedDeviceClassString(vr::ETrackedDeviceClass td_class) {

	string str_td_class = "Unknown class";

	switch (td_class)
	{
	case TrackedDeviceClass_Invalid:			// = 0, the ID was not valid.
		str_td_class = "Invalid";
		break;
	case TrackedDeviceClass_HMD:				// = 1, Head-Mounted Displays
		str_td_class = "HMD";
		break;
	case TrackedDeviceClass_Controller:			// = 2, Tracked controllers
		str_td_class = "Controller";
		break;
	case TrackedDeviceClass_GenericTracker:		// = 3, Generic trackers, similar to controllers
		str_td_class = "Generic Tracker";
		break;
	case TrackedDeviceClass_TrackingReference:	// = 4, Camera and base stations that serve as tracking reference points
		str_td_class = "Tracking Reference";
		break;
	case TrackedDeviceClass_DisplayRedirect:	// = 5, Accessories that aren't necessarily tracked themselves, but may redirect video output from other tracked devices
		str_td_class = "Display Redirecd";
		break;
	}

	return str_td_class;
}

//-----------------------------------------------------------------------------
// Purpose: Processes a single VR event
//-----------------------------------------------------------------------------
void SceneManager::process_vr_event(const vr::VREvent_t & event)
{
	string str_td_class = GetTrackedDeviceClassString(vr_context->GetTrackedDeviceClass(event.trackedDeviceIndex));

	switch(event.eventType)
	{
	case vr::VREvent_TrackedDeviceActivated:
		{
			cout << "Device " << event.trackedDeviceIndex << " attached (" << str_td_class << "). Setting up render model" << endl;

			// Load render models for the tracking device (when it's powered on duriing application execution)
			setup_render_model(event.trackedDeviceIndex);
		}
		break;
	case vr::VREvent_TrackedDeviceDeactivated:
		{
			cout << "Device " << event.trackedDeviceIndex << " detached (" << str_td_class << "). Removing render model" << endl;
			
			RenderModel* render_model = render_models[event.trackedDeviceIndex];
			render_models[event.trackedDeviceIndex] = NULL;

			delete render_model;
		}
		break;
	case vr::VREvent_TrackedDeviceUpdated:
		{
			cout << "Device " << event.trackedDeviceIndex << " updated (" << str_td_class << ")" << endl;
		}
		break;
	case vr::VREvent_ButtonPress:
		{
			VREvent_Controller_t controller_data = event.data.controller;
			cout << "Pressed button " << vr_context->GetButtonIdNameFromEnum((EVRButtonId) controller_data.button) << " of device " << event.trackedDeviceIndex << " (" << str_td_class << ")" << endl;

			// Another way of accessing the state of the controller...
			VRControllerState_t controller_state;
			TrackedDevicePose_t td_pose;
			if (vr_context->GetControllerStateWithPose(ETrackingUniverseOrigin::TrackingUniverseStanding,event.trackedDeviceIndex,&controller_state,sizeof(controller_state),&td_pose)) {
				if ((ButtonMaskFromId(EVRButtonId::k_EButton_Axis1) & controller_state.ulButtonPressed) != 0) {
					cout << "Trigger button pressed!" << endl;
					cout << "Pose information" << endl;
					cout << "  Tracking result: " << td_pose.eTrackingResult << endl;
					cout << "  Tracking velocity: (" << td_pose.vVelocity.v[0] << "," << td_pose.vVelocity.v[1] << "," << td_pose.vVelocity.v[2] << ")" << endl;
				}
			}
		}
		break;
	case vr::VREvent_ButtonUnpress:
		{
			VREvent_Controller_t controller_data = event.data.controller;
			cout << "Unpressed button " << vr_context->GetButtonIdNameFromEnum((EVRButtonId) controller_data.button) << " of device " << event.trackedDeviceIndex << " (" << str_td_class << ")" << endl;
		}
		break;
	case vr::VREvent_ButtonTouch:
		{
			VREvent_Controller_t controller_data = event.data.controller;
			cout << "Touched button " << vr_context->GetButtonIdNameFromEnum((EVRButtonId) controller_data.button) << " of device " << event.trackedDeviceIndex << " (" << str_td_class << ")" << endl;
		}
		break;
	case vr::VREvent_ButtonUntouch:
		{
			VREvent_Controller_t controller_data = event.data.controller;
			cout << "Untouched button " << vr_context->GetButtonIdNameFromEnum((EVRButtonId) controller_data.button) << " of device " << event.trackedDeviceIndex << " (" << str_td_class << ")" << endl;
		}
		break;
	}
}

void SceneManager::update() {

	if (vr_context != NULL)
	{
		// Process SteamVR events
		VREvent_t vr_event;
		while(vr_context->PollNextEvent(&vr_event,sizeof(vr_event)))
		{
			process_vr_event(vr_event);
		}

		// Update tracking device poses...
		VRCompositor()->WaitGetPoses(tracked_device_pose, k_unMaxTrackedDeviceCount, NULL, 0 );

		for (int nDevice=0; nDevice<k_unMaxTrackedDeviceCount; nDevice++)
		{
			if (tracked_device_pose[nDevice].bPoseIsValid)
			{
				tracked_device_pose_matrix[nDevice] = convert_SteamVRMat_to_GLMMat(tracked_device_pose[nDevice].mDeviceToAbsoluteTracking);
			}
		}

		if (tracked_device_pose[k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		{
			hmd_pose_matrix = tracked_device_pose_matrix[k_unTrackedDeviceIndex_Hmd];
		}
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type)
		{
		case SDL_KEYDOWN:
			{ 
				if (event.key.keysym.sym == SDLK_ESCAPE) {
					cout << "Exiting app..." << endl;
					app_end = true;
				}
			}
			break;
		case SDL_QUIT: // In case the user presses the window's X button we malso must properly shutdown down the app
			{
				cout << "Exiting app..." << endl;
				app_end = true;
			}
			break;
		}
	}

	// Update the model matrix to rotate the cube on the Y axis
	float time_2 = (float) SDL_GetTicks();
	model *= glm::rotate(glm::mat4(1.f),((time_2-time_1)/1000)*glm::radians(50.0f),glm::vec3(0.0f, 1.0f, 0.0f));	// Rotate along Y
	time_1 = time_2;
}

void SceneManager::draw() {

	glClearColor(1.f,1.f,1.f,1.0f);

	//////////////////////////////////////////////
	// Render left eye...

	glBindFramebuffer(GL_FRAMEBUFFER, l_frame_buffer);
	glViewport(0, 0, render_width, render_height);
	glEnable(GL_DEPTH_TEST);

	// Make our background white
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scene_shader->use_program();
	scene_shader->set_mvp_matrix(projection_matrix_left*view_matrix_left*glm::inverse(hmd_pose_matrix)*model); // If we don't apply the hmd_pose_matrix^-1 here the cube will follow the HMD position (kinda UI)

	glBindVertexArray(cube_texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cube_texture);
	
	// Draw top cube to frame buffer
	glDrawArrays(GL_TRIANGLES, 0, 30);

	glBindVertexArray(0);

	for (int rm_id=0; rm_id<vr::k_unMaxTrackedDeviceCount; rm_id++)
	{
		if (render_models[rm_id] != NULL)
		{
			render_model_shader->use_program();
			render_model_shader->set_mvp_matrix(projection_matrix_left*view_matrix_left*glm::inverse(hmd_pose_matrix)*tracked_device_pose_matrix[rm_id]);
			//glUniform1i(glGetUniformLocation(render_model_shader->get_shader(),"tex"), 0); // I guess this is not neccessary as it binds texture #0 to the shader parameter by default...

			render_models[rm_id]->draw();
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//////////////////////////////////////////////
	// Render right eye...

	glBindFramebuffer(GL_FRAMEBUFFER, r_frame_buffer);
	glViewport(0, 0, render_width, render_height);
	glEnable(GL_DEPTH_TEST);

	// Make our background white
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	scene_shader->use_program();
	scene_shader->set_mvp_matrix(projection_matrix_right*view_matrix_right*glm::inverse(hmd_pose_matrix)*model); // If we don't apply the hmd_pose_matrix^-1 here the cube will follow the HMD position (kinda UI)

	glBindVertexArray(cube_texture);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, cube_texture);
	
	// Draw top cube to frame buffer
	glDrawArrays(GL_TRIANGLES, 0, 30);

	glBindVertexArray(0);

	for (int rm_id=0; rm_id<vr::k_unMaxTrackedDeviceCount; rm_id++)
	{
		if (render_models[rm_id] != NULL)
		{
			render_model_shader->use_program();
			render_model_shader->set_mvp_matrix(projection_matrix_right*view_matrix_right*glm::inverse(hmd_pose_matrix)*tracked_device_pose_matrix[rm_id]);
			//glUniform1i(glGetUniformLocation(render_model_shader->get_shader(),"tex"), 0); // I guess this is not neccessary as it binds texture #0 to the shader parameter by default...

			render_models[rm_id]->draw();
		}
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Submit render buffers to HMD

	Texture_t leftEyeTexture = {(void*)(uintptr_t) l_tex_color_buffer, TextureType_OpenGL, ColorSpace_Gamma};
	VRCompositor()->Submit(Eye_Left, &leftEyeTexture);
	Texture_t rightEyeTexture = {(void*)(uintptr_t) r_tex_color_buffer, TextureType_OpenGL, ColorSpace_Gamma};
	VRCompositor()->Submit(Eye_Right, &rightEyeTexture);

	/////////////////////////////////////////////
	// Render what companion window will show
	// In this case we render what left eye is actually seeing

	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, w_width, w_height);

	screen_shader->use_program();

	// Bind default framebuffer and draw contents of our framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindVertexArray(square_texture_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,square_texture_ebo);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, l_tex_color_buffer); // Do mind that calls like glBindTexture which change the OpenGL state are relatively expensive, so we must try to keeping them to a minimum
	glUniform1i(glGetUniformLocation(screen_shader->get_shader(),"frame_buffer_tex"),0);

    glDrawElements(GL_TRIANGLES, sizeof(elements), GL_UNSIGNED_INT, 0);
	
	glUseProgram(0);
	glBindVertexArray(0);

	// Swap our buffers to make our changes visible
	SDL_GL_SwapWindow(companion_window);

	// Flush and wait for swap
	if (v_blank)
	{
		glFlush();
		glFinish();
	}
}

void SceneManager::exit() {

	// Delete our OpengL context
	SDL_GL_DeleteContext(gl_context);
	// Destroy our window
	SDL_DestroyWindow(companion_window);
	companion_window = NULL;
	// Shutdown SDL2
	SDL_Quit();

	// FIXME: this is returning an error when exiting...
	/*delete scene_shader;
	delete render_model_shader;
	delete screen_shader;*/

	for (int rm_id=0; rm_id<k_unMaxTrackedDeviceCount; rm_id++)
	{
		if (render_models[rm_id] != NULL)
		{
			delete render_models[rm_id];
			render_models[rm_id] = NULL;
		}
	}

	glDeleteBuffers(1,&square_texture_vbo);
	glDeleteBuffers(1,&square_texture_ebo);
	glDeleteVertexArrays(1,&square_texture_vao);

	glDeleteTextures(1, &cube_texture);
	glDeleteBuffers(1,&cube_texture_vbo);
	glDeleteVertexArrays(1,&cube_texture_vao);

	glDeleteFramebuffers(1, &l_frame_buffer);
	glDeleteRenderbuffers(1, &l_render_buffer_depth_stencil);

	glDisableVertexAttribArray(0);

	VR_Shutdown();
}

int SceneManager::initOpenVR() {

	if (VR_IsHmdPresent())
	{
		cout << "An HMD was successfully found in the system" << endl;

		if (VR_IsRuntimeInstalled()) {
			const char* runtime_path = VR_RuntimePath();
			cout << "Runtime correctly installed at '" << runtime_path << "'" << endl;
		}
		else
		{
			cout << "Runtime was not found, quitting app" << endl;
			return -1;
		}
	}
	else
	{
		cout << "No HMD was found in the system, quitting app" << endl;
		return -1;
	}

	// Load the SteamVR Runtime
	HmdError err;
	vr_context = VR_Init(&err,EVRApplicationType::VRApplication_Scene);
	vr_context == NULL ? cout << "Error while initializing VRSystem. Error code is " << VR_GetVRInitErrorAsSymbol(err) << endl : cout << "VRSystem successfully initialized" << endl;

	// Load render models. Afaik this is used just to load generic render models for this implementation,
	// like base stations, controllers, sensors, etc. (render_models is not explictly used anymore)
	vr_render_models = (vr::IVRRenderModels *)vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &err);
	if (!vr_render_models)
	{
		vr::VR_Shutdown();
		cout << "Couldn't load generic render models" << endl;
		return -1;
	}
	else
		cout << "Render models loaded successfully" << endl;

	for (uint32_t td=vr::k_unTrackedDeviceIndex_Hmd; td<k_unMaxTrackedDeviceCount; td++) {

		if (vr_context->IsTrackedDeviceConnected(td))
		{
			ETrackedDeviceClass tracked_device_class = vr_context->GetTrackedDeviceClass(td);

			cout << "Tracking device " << td << " is connected " << endl;
			cout << "  Device type: " << GetTrackedDeviceClassString(tracked_device_class) << ". Name: " << GetTrackedDeviceString(vr_context,td,vr::Prop_TrackingSystemName_String) << endl;

			if (tracked_device_class == ETrackedDeviceClass::TrackedDeviceClass_TrackingReference) base_stations_count++;

			if (td == k_unTrackedDeviceIndex_Hmd)
			{
				// Fill variables used for obtaining the device name and serial ID (used later for naming the SDL window)
				m_strDriver = GetTrackedDeviceString(vr_context,vr::k_unTrackedDeviceIndex_Hmd,vr::Prop_TrackingSystemName_String);
				m_strDisplay = GetTrackedDeviceString(vr_context,vr::k_unTrackedDeviceIndex_Hmd,vr::Prop_SerialNumber_String);
			}
			else
				if (!setup_render_model(td))
					return -1;
		}
		else
			cout << "Tracking device " << td << " not connected" << endl;
	}

	// Check whether both base stations are found
	if (base_stations_count < 2)
	{
		cout << "There was a problem indentifying the base stations, please check they are powered on" << endl;
		return -1;
	}

	if (!(vr_compositor = VRCompositor()))
	{
		cout << "Compositor initialization failed. See log file for details" << endl;
		return -1;
	}
	else 
		cout << "Compositor successfully initialized" << endl;

	// Setup left and right eye frame buffers
	vr_context->GetRecommendedRenderTargetSize(&render_width, &render_height);
	cout << "Recommended render targer size is " << render_width << "x" << render_height << endl;

	cout << "Setting up left eye frame buffer..." << endl;
	if (!setup_frame_buffer(render_width,render_height,l_frame_buffer,l_render_buffer_depth_stencil,l_tex_color_buffer)) return -1;
	cout << "Left eye frame buffer setup completed!" << endl;

	cout << "Setting up right eye frame buffer..." << endl;
	if (!setup_frame_buffer(render_width,render_height,r_frame_buffer,r_render_buffer_depth_stencil,r_tex_color_buffer)) return -1;
	cout << "Right eye frame buffer setup completed!" << endl;

	// Obtain projection and view-eye matrices for the HMD
	// Notes: We initially had this calculation as part of the update loop and SteamVR reprojected almost every frame o.O. This was solved calculating the HMD projection and view matrices only
	//		  at setup. Another optimization was to start passing the whole mvp matrix to the different shaders (calculating the product on CPU side) instead of the projection, view and model
	//		  matrices individually (in the latter case the projection*view*model calculation must be done at vertex shader level for EVERY vertex in the geometry being rendered and may add some
	//		  overhead despite being done on the GPU side)

	projection_matrix_left = get_projection_matrix(Hmd_Eye::Eye_Left);
	projection_matrix_right = get_projection_matrix(Hmd_Eye::Eye_Right);
	view_matrix_left = get_view_matrix(Hmd_Eye::Eye_Left);
	view_matrix_right = get_view_matrix(Hmd_Eye::Eye_Right);

	return 0;
}

int SceneManager::initSDL() {

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // Enable to tell SDL that the old, deprecated GL code are disabled, only the newer versions can be used
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION,3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION,1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	/*SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 0);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 0);*/

	companion_window = SDL_CreateWindow("Hello world VR", w_pos_x, w_pos_y, w_width, w_height, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (companion_window == NULL) {
		SDL_Log("Unable to create SDL Window: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	gl_context = SDL_GL_CreateContext(companion_window);
	if (gl_context == NULL) {
		SDL_Log("Unable to create GL Context: %s", SDL_GetError());
		SDL_Quit();
		return -1;
	}

	if (SDL_GL_SetSwapInterval(v_blank ? 1 : 0) < 0) // Configure VSync
	{
		SDL_Log("Warning: Unable to set VSync!: %s", SDL_GetError());
		return -1;
	}

	glewExperimental = GL_TRUE;		// Enable to tell OpenGL that we want to use OpenGL 3.0 stuff and later
	GLenum nGlewError = glewInit(); // Initialize glew
	if (nGlewError != GLEW_OK)
	{
		SDL_Log( "Error initializing GLEW! %s", glewGetErrorString( nGlewError ) );
		return -1;
	}

	string strWindowTitle = "openvr_main - " + m_strDriver + " " + m_strDisplay;
	SDL_SetWindowTitle(companion_window, strWindowTitle.c_str());

	return 0;
}

int SceneManager::initOpenGL() {

	int success = 0;
    GLenum error = GL_NO_ERROR;

	// Initialize clear color
    glClearColor(0.f,0.f,0.f,1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if((error = glGetError()) != GL_NO_ERROR)
    {
        cout << "Error initializing OpenGL!" << gluErrorString(error) << endl;
		return -1;
    }

	glViewport(0, 0, w_width, w_height);

	// Enable depth testing so that closer triangles will hide the triangles farther away
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	cout << "Setting up scene VBO+VAO..." << endl;
	success = setup_scene_buffers() ? 0 : -1;
	cout << "Scene VBO+VAO setup completed!" << endl;

	if (render_model_shader->init())
		cout << "Render model shader successfully initialized" << endl;
	else
	{
		cout << "Coulnd't initialize render model shader" << endl;
		return -1;
	}

	return success;
}

bool SceneManager::setup_scene_buffers() {

	// Create and bind a vertex array for storing both cube attributes (positions and colors)
	glGenVertexArrays(1,&cube_texture_vao);
	glBindVertexArray(cube_texture_vao);

	// Create and bind a vertex buffer for cube vertex positions and copy the data from the cube vertices
	glGenBuffers(1,&cube_texture_vbo);
	glBindBuffer(GL_ARRAY_BUFFER,cube_texture_vbo);
	glBufferData(GL_ARRAY_BUFFER,sizeof(cube_tex),cube_tex,GL_STATIC_DRAW);

	// Create a texture buffer
	glGenTextures(1,&cube_texture);

	// TEXTURE 1...

	glBindTexture(GL_TEXTURE_2D,cube_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Load texture image
	int width, height;
	unsigned char* image = SOIL_load_image(".\\.\\portal_cube.jpg", &width, &height, 0, SOIL_LOAD_RGB);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

	glGenerateMipmap(GL_TEXTURE_2D);

	SOIL_free_image_data(image);
	
	// Create and bind a vertex array for storing square attributes (positions, colors and textures)
	// In this square we are going to map the texture from the frame buffer containing the scene
	glGenVertexArrays(1,&square_texture_vao);
	glBindVertexArray(square_texture_vao);

	// Create and bind a vertex buffer for square vertex positions and copy the data from the square vertices
	glGenBuffers(1,&square_texture_vbo);
	glBindBuffer(GL_ARRAY_BUFFER,square_texture_vbo);
	glBufferData(GL_ARRAY_BUFFER,vertices*floatsPerVertex*sizeof(GLfloat),square_tex,GL_STATIC_DRAW);

	// Create and bind an element buffer for square vertex indexes and copy the data from the square vertices
	glGenBuffers(1,&square_texture_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,square_texture_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(elements),elements,GL_STATIC_DRAW);

	cout << "Initializing shaders..." << endl;
	
	if (scene_shader->init())
	{
		scene_shader->use_program();

		glBindVertexArray(cube_texture);
		glBindBuffer(GL_ARRAY_BUFFER, cube_texture);

		// Specify the layout of the vertex data
		GLint posAttrib = glGetAttribLocation(scene_shader->get_shader(),"in_Position");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, floatsPerVertex*sizeof(GLfloat), (GLvoid*) 0);

		GLint colAttrib = glGetAttribLocation(scene_shader->get_shader(),"in_Color");
		glEnableVertexAttribArray(colAttrib);
		glVertexAttribPointer(colAttrib, 4, GL_FLOAT, GL_FALSE, floatsPerVertex*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));

		GLint texAttrib = glGetAttribLocation(scene_shader->get_shader(),"in_Texcoord");
		glEnableVertexAttribArray(texAttrib);
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, floatsPerVertex*sizeof(GLfloat), (GLvoid*)(7*sizeof(GLfloat)));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,cube_texture);
		glUniform1i(glGetUniformLocation(scene_shader->get_shader(),"tex"),0);

		cout << "Scene shaders successfully initialized" << endl;
	}
	else
		return false;

	if (screen_shader->init())
	{
		screen_shader->use_program();

		glBindVertexArray(square_texture_vao);
		glBindBuffer(GL_ARRAY_BUFFER, square_texture_vbo);

		// Specify the layout of the vertex data
		GLint posAttrib = glGetAttribLocation(screen_shader->get_shader(),"in_Position");
		glEnableVertexAttribArray(posAttrib);
		glVertexAttribPointer(posAttrib, 3, GL_FLOAT, GL_FALSE, floatsPerVertex*sizeof(GLfloat), (GLvoid*) 0);

		GLint colAttrib = glGetAttribLocation(screen_shader->get_shader(),"in_Color");
		glEnableVertexAttribArray(colAttrib);
		glVertexAttribPointer(colAttrib, 4, GL_FLOAT, GL_FALSE, floatsPerVertex*sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));

		GLint texAttrib = glGetAttribLocation(screen_shader->get_shader(),"in_Texcoord");
		glEnableVertexAttribArray(texAttrib);
		glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, floatsPerVertex*sizeof(GLfloat), (GLvoid*)(7*sizeof(GLfloat)));

		cout << "Screen shaders successfully initialized" << endl;
	}
	else
		return false;

	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind EBO
	glBindVertexArray(0); // Unbind VAO

	return true;
}

bool SceneManager::setup_frame_buffer(GLsizei width, GLsizei height, GLuint &frame_buffer, GLuint &render_buffer_depth_stencil, GLuint &tex_color_buffer) {

	// Create and bind the frame buffer
	glGenFramebuffers(1, &frame_buffer);
	glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer);

	// Create texture where we are going to bulk the contents of the frame buffer
	glGenTextures(1, &tex_color_buffer);
	glBindTexture(GL_TEXTURE_2D, tex_color_buffer);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Attach the image to the framebuffer
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex_color_buffer, 0); // The second parameter implies that you can have multiple color attachments. A fragment shader can output
																									  // different data to any of these by linking 'out' variables to attachments with the glBindFragDataLocation function

	// Create the render buffer to host the depth and stencil buffers
	// Although we could do this by creating another texture, it is more efficient to store these buffers in a Renderbuffer Object, because we're only interested in reading the color buffer in a shader
	glGenRenderbuffers(1, &render_buffer_depth_stencil);
	glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_depth_stencil);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

	// Attach the render buffer to the framebuffer
	// NOTE: Even if you don't plan on reading from this depth_attachment, an off screen buffer that will be rendered to should have a depth attachment
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, render_buffer_depth_stencil);

	// Check whether the frame buffer is complete (at least one buffer attached, all attachmentes are complete, all attachments same number multisamples)
	(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) ?	cout << "The frame buffer is complete!" << endl : cout << "The frame buffer is invalid, please re-check. Code: " << glCheckFramebufferStatus(l_frame_buffer) << endl;

	return true;
}

bool SceneManager::setup_render_model(vr::TrackedDeviceIndex_t tracked_device) {

	if (tracked_device >= k_unMaxTrackedDeviceCount)
		return false;

	string render_model_name = GetTrackedDeviceString(vr_context, tracked_device, vr::Prop_RenderModelName_String);
	
	render_models[tracked_device] = new RenderModel(render_model_name.c_str());

	cout << "Starting initialization of render model data for device " << tracked_device << endl;
	if (render_models[tracked_device]->init())
	{
		cout << "Render model for device " << tracked_device << " correctly initialized" << endl;
		return true;
	}

	cout << "Unable to generate render model data for device " << tracked_device << endl;
	return false;
}

glm::mat4 SceneManager::convert_SteamVRMat_to_GLMMat(const HmdMatrix34_t &matPose) {

	glm::mat4 pose_matrix = glm::mat4(matPose.m[0][0],matPose.m[1][0],matPose.m[2][0],0.0f,
									  matPose.m[0][1],matPose.m[1][1],matPose.m[2][1],0.0f,
									  matPose.m[0][2],matPose.m[1][2],matPose.m[2][2],0.0f,
									  matPose.m[0][3],matPose.m[1][3],matPose.m[2][3],1.0f); // As glm stores matrices in column major order and HmdMatrix34 are in row major order, ther first 3 elements of pose_matrix
																							 // should be the x.x, y.x and z.x coords which corresponds to HmdMatrix34[0][0], HmdMatrix34[1][0], HmdMatrix34[2][0]
																							 // The last row of the pose_matrix should be the position of the coordinate system, (0,0,0,1) in this case
																							 // More info. abour matrix order in OpenVR and glm here: https://github.com/ValveSoftware/openvr/issues/193

	return pose_matrix;
}

glm::mat4 SceneManager::get_projection_matrix(const Hmd_Eye eye)
{
	// We should always use the OpenVR projection matrix in order to avoid nausea and
	// sickness (we can't change fov adn stuff): https://steamcommunity.com/app/358720/discussions/0/359543542248432114/
	// Here's another interesting link on the topic: http://rifty-business.blogspot.com.uy/2013/10/understanding-matrix-transformations.html

	HmdMatrix44_t steamvr_proj_matrix = vr_context->GetProjectionMatrix(eye,0.1f,15.f);

	return glm::mat4(steamvr_proj_matrix.m[0][0],steamvr_proj_matrix.m[1][0],steamvr_proj_matrix.m[2][0],steamvr_proj_matrix.m[3][0],
					 steamvr_proj_matrix.m[0][1],steamvr_proj_matrix.m[1][1],steamvr_proj_matrix.m[2][1],steamvr_proj_matrix.m[3][1],
					 steamvr_proj_matrix.m[0][2],steamvr_proj_matrix.m[1][2],steamvr_proj_matrix.m[2][2],steamvr_proj_matrix.m[3][2],
					 steamvr_proj_matrix.m[0][3],steamvr_proj_matrix.m[1][3],steamvr_proj_matrix.m[2][3],steamvr_proj_matrix.m[3][3]);
}

glm::mat4 SceneManager::get_view_matrix(const Hmd_Eye eye)
{
	HmdMatrix34_t steamvr_eye_view_matrix = vr_context->GetEyeToHeadTransform(eye);

	glm::mat4 view_matrix = glm::mat4(steamvr_eye_view_matrix.m[0][0],steamvr_eye_view_matrix.m[1][0],steamvr_eye_view_matrix.m[2][0],0.0f,
									  steamvr_eye_view_matrix.m[0][1],steamvr_eye_view_matrix.m[1][1],steamvr_eye_view_matrix.m[2][1],0.0f,
									  steamvr_eye_view_matrix.m[0][2],steamvr_eye_view_matrix.m[1][2],steamvr_eye_view_matrix.m[2][2],0.0f,
									  steamvr_eye_view_matrix.m[0][3],steamvr_eye_view_matrix.m[1][3],steamvr_eye_view_matrix.m[2][3],1.0f);

	return glm::inverse(view_matrix);
}