
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <windows.h>
#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#define STB_IMAGE_IMPLEMENTATION

#include "Constants.hpp"
#include "Shader.hpp"
#include "ShaderProgram.hpp"
#include "Vec.hpp"
#include "Camera.hpp"
#include "Helpers.hpp"
#include "Texture.hpp"
#include "Model.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include "ModelLoader.hpp"
#include "DynamicCubeMap.hpp"
#include "MeshBuilder.hpp"
#include "GameObject.hpp"
#include <memory>
#include "Visualization.hpp"
#include "Voxelizer.hpp"
#include "CollisionDetector.hpp"
#include "PhysicsSolver.hpp"
#include "WaterGrid.hpp"
#include "reflectionTexture.hpp"

shared_ptr<Camera> camera;
shared_ptr<GameObject> selected;

bool camera_control = false;
bool mouse_dragging = false;
float player_movement_speed = 2.5f;
std::function<void(int)> add_ball;
struct PointLight {
	float position[3];
	float ambient[3];
	float diffuse[3];
	float specular[3];
	float attenuation[3];
	bool enable;
};
vector<PointLight> point_lights;
bool pause_world = true;
bool view_collision = false;
bool view_points = true;
bool view_aabb = false;
bool view_trajectory = true;
int trajectory_length = 600; // frames
bool reset_flag = false;
shared_ptr<GameObject> selected_forAdjustment;

const int mx_time_record = 100000;
int time_steps = 0;
int cur_time = 0;
int time_front = 0;

bool show_imgui = true;
vector<GameObject> obj_states[mx_time_record];
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_E && action == GLFW_PRESS){
		camera_control = !camera_control;
		glfwSetInputMode(window, GLFW_CURSOR, camera_control ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
	if (key == GLFW_KEY_P && action == GLFW_PRESS) {
		pause_world = !pause_world;
	}
	if (key == GLFW_KEY_R && action == GLFW_PRESS) {
		reset_flag = true;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		cur_time = ((cur_time - 1) % mx_time_record + mx_time_record) % mx_time_record;
	}
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		cur_time = (cur_time + 1) % mx_time_record;
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		exit(0);
	}
	if(key == GLFW_KEY_G && action == GLFW_PRESS){
		show_imgui = !show_imgui;
	}
	
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		mouse_dragging = true;
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		selected_forAdjustment = selected;
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		mouse_dragging = false;
	}
}

void updateWorld(GLFWwindow* window, float delta) {
	float camera_speed = player_movement_speed * delta;
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		camera->position += camera_speed * -camera->view.transposed().z_axis();
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		camera->position -= camera_speed * -camera->view.transposed().z_axis();
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera->position += camera_speed * -camera->view.transposed().x_axis();
	}
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		camera->position -= camera_speed * -camera->view.transposed().x_axis();
	}
	if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
		add_ball(1);
	}

	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
		selected_forAdjustment->addForceAtPoint(selected_forAdjustment->mass * vec3(0, -9.8, 0), (selected_forAdjustment->toWorld()).inverse() * vec3(-1, 0.3, -1));
	}
	camera->updateView();

	float R = 10.0f;
	point_lights[2].position[0] = R * cos(glfwGetTime() * 0.1) + R;
	point_lights[2].position[2] = R * sin(glfwGetTime() * 0.1) + R;
}

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
const unsigned int ENV_WIDTH = 1024, ENV_HEIGHT = 1024;
int window_width = WINDOW_WIDTH, window_height = WINDOW_HEIGHT;

vec3 world_mouse;
vec3 far_world_mouse;
vec3 mmpos;
bool valid_dragging = false;
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	if (selected) {
		auto ray_direct = uni(far_world_mouse - world_mouse);
		if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
			selected->orientation = selected->orientation.rotate(ray_direct * 0.1 * yoffset);
			return;
		}
		selected->position += 0.5 * yoffset * ray_direct;
		selected->initialize_lastframe();
	}
}
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	static double last_x = xpos, last_y = ypos;
	double dx = xpos - last_x, dy = ypos - last_y;
	last_x = xpos, last_y = ypos;
	if(camera_control)
	camera->mouseMove(dx, -dy);

	float mx = xpos;
	float my = window_height - ypos; 
	mx = (mx - window_width / 2.0) * (2.0 / window_width);
	my = (my - window_height / 2.0) * (2.0 / window_height);

	world_mouse = camera->getWorldMousePos(mx,my,0);
	far_world_mouse = camera->getWorldMousePos(mx, my, 1);
	if (selected && mouse_dragging) {
		valid_dragging = true;
		auto normal = uni( - camera->view.transposed().z_axis());
		auto pos = selected->position;
		auto mpos = world_mouse;
		auto o = world_mouse;
		auto ray_direct = uni(far_world_mouse - world_mouse);
		auto denom = ray_direct * normal;
		if (denom > eps) {
			float nume = (pos - mpos) * normal;
			float t = nume / denom;
			if (t >= 0.0f) {
				selected->position = t * ray_direct + mpos;
				selected->initialize_lastframe();
				mmpos = t * ray_direct + mpos;
			}
		}
	}
	else {
		valid_dragging = false;
	}
}

bool showing_depth_map = 0;
bool enable_acc_structure = 1;
int ball_batch = 1;
float ball_speed_amplifiler = 1.0;
bool split_window = false;
int mx_time_front = 0;
shared_ptr<WaterGrid> water_grid;
PhysicsSolver physic_solver;
float distance_visualization_slice = 0.5f;
bool view_collision_points = false;
void render_ui(float fps) {
	if (!show_imgui) return;
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Light Configurator");
	ImGui::Text(("fps:" + std::to_string(fps)).c_str());
	ImGui::Checkbox("showing depth map", &showing_depth_map);
	ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
	for (int i = 0; i < point_lights.size(); ++i) {
		if (ImGui::TreeNodeEx(("Point Light #" + std::to_string(i)).c_str(), 0)) {
			ImGui::ColorEdit3("ambient", point_lights[i].ambient);
			ImGui::ColorEdit3("diffuse", point_lights[i].diffuse);
			ImGui::ColorEdit3("specular", point_lights[i].specular);
			ImGui::InputFloat3("attenuation", point_lights[i].attenuation);
			ImGui::SliderFloat("pos.x", &(point_lights[i].position[0]),-10, 10);
			ImGui::SliderFloat("pos.y", &(point_lights[i].position[1]),-10, 10);
			ImGui::SliderFloat("pos.z", &(point_lights[i].position[2]),-10, 10);
			ImGui::Checkbox("enable", &point_lights[i].enable);
			ImGui::TreePop();
		}
	}
	ImGui::SliderFloat("Wave Limitation",&water_grid->wave_limit, 0, 5.0f);
	ImGui::SliderFloat("Wave Amplifier", &water_grid->wave_amplifier, 0, 5.0f);

	ImGui::SliderInt("Grid Size", &water_grid->grid_resolution, 1, 900);

	ImGui::SliderInt("Padding", &water_grid->padding, 0, 900);
	ImGui::SliderFloat("Damping", &water_grid->damping, 0.0, 1.0f);
	ImGui::SliderFloat("Distance Visualize Slice", &distance_visualization_slice, 0.0f, 1.0f);
	ImGui::SliderInt("Time", &cur_time, 0 , mx_time_front);

	ImGui::Checkbox("View AABB", &view_aabb);
	ImGui::Checkbox("View Collision", &view_collision);
	ImGui::Checkbox("View Collision Points", &view_collision_points);
	ImGui::Checkbox("View Parameter Points", &view_points);
	ImGui::Checkbox("View Trajectory", &view_trajectory);
	ImGui::SliderInt("Trajectory length", &trajectory_length, 2, 5000);


	ImGui::Text("Contact generation");
	ImGui::RadioButton("Distance field", &physic_solver.collision_detector->contact_mode, CONTACT_DISTANCE_FIELD); ImGui::SameLine();
	ImGui::RadioButton("Voxel", &physic_solver.collision_detector->contact_mode, CONTACT_VOXEL);
	ImGui::Text("contacts this frame: %d", (int)physic_solver.collision_detector->collisions.size());

	ImGui::SliderInt("descent_iterations", &physic_solver.collision_detector->descent_iterations, 0, 400);
	ImGui::SliderFloat("descent_step", &physic_solver.collision_detector->descent_step, 0.0f, 0.001f,"%.6f");

	if (selected_forAdjustment) {
		ImGui::SliderFloat("Mass", &selected_forAdjustment->mass, -5.0, 5000.0f);
		ImGui::SliderFloat("Velocity.x", &selected_forAdjustment->velocity.x, -5.0f, 5.0f);
		ImGui::SliderFloat("Velocity.y", &selected_forAdjustment->velocity.y, -5.0f, 5.0f);
		ImGui::SliderFloat("Velocity.z", &selected_forAdjustment->velocity.z, -5.0f, 5.0f);

		ImGui::SliderFloat("position.x", &selected_forAdjustment->position.x, -5.0f, 5.0f);
		ImGui::SliderFloat("position.y", &selected_forAdjustment->position.y, -5.0f, 5.0f);
		ImGui::SliderFloat("position.z", &selected_forAdjustment->position.z, -5.0f, 5.0f);

		ImGui::SliderFloat("orientation.w", &selected_forAdjustment->orientation.w, -5.0f, 5.0f);
		ImGui::SliderFloat("orientation.i", &selected_forAdjustment->orientation.i, -5.0f, 5.0f);
		ImGui::SliderFloat("orientation.j", &selected_forAdjustment->orientation.j, -5.0f, 5.0f);
		ImGui::SliderFloat("orientation.k", &selected_forAdjustment->orientation.k, -5.0f, 5.0f);

	}
	//ImGui::Checkbox("Accelerated Structure",&enable_acc_structure);
	//ImGui::SliderFloat("Ball Initial Speed", &ball_speed_amplifiler, 0.0, 20.0);

	ImGui::Checkbox("Split View", &split_window);
	/*if (ImGui::Button("Add ball to Scence")) {
		std::cout << "added balls : " << ball_batch << "\n";
		add_ball(ball_batch);
	}*/
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

float d_width = window_width / 2.0;
float d_height = window_height / 2.0;

void onResize(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
	if (split_window)
		camera->windowResize(width / 2.0, height / 2.0);
	else {
		camera->windowResize(width / 2.0, height / 2.0);
	}
	d_width = width / 2.0;
	d_height = height / 2.0;
	window_width = width;
	window_height = height;
}
signed main() {

	camera = make_shared<Camera>();
	camera->position = vec3(-7.13026619, -3.15038109, 6.73415852);
	camera->yx = 3.98099971;
	camera->rx = -0.599999785;
	camera->windowResize(window_width, window_height);

#pragma region WindowInitialization
	GLFWwindow* window;

	if (!glfwInit()) return -1;

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Phong Shade", NULL, NULL);

	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, onResize);

	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSwapInterval(1); // Make fps constants
#pragma endregion
#pragma region GladInitialization

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Fail to load glad\n";
		return 0;
	}
	std::cout << glGetString(GL_VERSION) << '\n';

#pragma endregion
#pragma region ImGuiSetup
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 430");
#pragma endregion

	glViewport(0, 0, window_width, window_height);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_PACK_ALIGNMENT, 1);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	/*glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar* message, const void* userParam) {
			fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
				(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
				type, severity, message);
		}, 0);*/

	shared_ptr<ShaderProgram> shader_program = make_shared<ShaderProgram>
		(vector<shared_ptr<Shader>>{ 
		make_shared<Shader>("phong.vert", GL_VERTEX_SHADER), 
		make_shared<Shader>("phong.frag", GL_FRAGMENT_SHADER),
	});
	shared_ptr<ShaderProgram> shadow_program = make_shared<ShaderProgram>
		(vector<shared_ptr<Shader>>{
		make_shared<Shader>("shadow.vert", GL_VERTEX_SHADER),
		make_shared<Shader>("shadow.frag", GL_FRAGMENT_SHADER),
		make_shared<Shader>("shadow.geom", GL_GEOMETRY_SHADER)
	});
	shared_ptr<ShaderProgram> refract_program = make_shared<ShaderProgram>
		(vector<shared_ptr<Shader>>{
		make_shared<Shader>("refract.vert", GL_VERTEX_SHADER),
		make_shared<Shader>("refract.frag", GL_FRAGMENT_SHADER)
	});

	shared_ptr<Texture> texture_white = make_shared<Texture>();
	shared_ptr<Texture> texture_blue = make_shared<Texture>(0xff0000);

	shared_ptr<Texture> texture_yellow = make_shared<Texture>(0x00ffff);
	shared_ptr<Texture> texture_green = make_shared<Texture>(0x00ff00);

	shared_ptr<Texture> toilet_texture = make_shared<Texture>("Toilet.jpg");
	shared_ptr<Texture> warning_tape = make_shared<Texture>("warningTape.png");
	shared_ptr<Texture> warp_tape = make_shared<Texture>("warp.jpg");
	shared_ptr<Texture> wood_floor = make_shared<Texture>("wood.png");

	shared_ptr<Texture> engraver = make_shared<Texture>(10, 10);

	Visualization visualizer(camera);
	shared_ptr<Voxelizer> voxelizer = make_shared<Voxelizer>();

	// Setup lights
	point_lights.push_back({
		{0,15,0}, //position
		{0.5,0.5,0.5}, //ambient
		{1,1,1}, //diffuse
		{.5,.5,.5}, //specular
		{0,0.05,1}, //attenuation
		1, //enable
	});
	point_lights.push_back({
		{2,6,1},
		{0,0,0},
		{1,0,0},
		{1,0,0},
		{0,0,1},
		0,
	});

	point_lights.push_back({
		{0,6, 0},
		{0,0,0},
		{1,1,1},
		{1,1,1},
		{0,0,1},
		1,
	});

	
	GLuint* depthmap = new GLuint[point_lights.size() + 1];
	GLuint* depthFBO = new GLuint[point_lights.size() + 1];
	glGenTextures(point_lights.size() + 1, depthmap);
	glGenFramebuffers(point_lights.size() + 1, depthFBO);
	for (int i = 0; i < point_lights.size() + 1; ++i) {
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthmap[i]);
		for (int i = 0; i < 6; ++i) {
			GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
			glTexImage2D(face, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
		}
		// Render the scence in a single pass 
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

		glBindFramebuffer(GL_FRAMEBUFFER, depthFBO[i]);
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depthmap[i], 0);
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	shader_program->use();
	shader_program->setVec3("material.ambient", vec3(.2));
	shader_program->setVec3("material.diffuse", vec3(.8));
	shader_program->setVec3("material.specular", vec3(1));
	shader_program->setFloat("material.shininess",32);

	shader_program->setFloat("opacity", 1.0f);
	
	shader_program->use();

	shared_ptr<ModelLoader> teapot_nolid_raw = make_shared<ModelLoader>("utah_teapot_nolid.obj");
	//shared_ptr<ModelLoader> toilet_raw = make_shared<ModelLoader>("10778_Toilet_V2.obj");
	//shared_ptr<ModelLoader> teapot_raw = make_shared<ModelLoader>("utah_teapot_nold.obj");
	//shared_ptr<Model> teapot = make_shared<Model>(teapot_raw->vertices, teapot_raw->vertex_size * 8 * 4, teapot_raw->vertex_size);
	shared_ptr<Model> teapot_nolid = make_shared<Model>(teapot_nolid_raw->vertices, teapot_nolid_raw->vertex_size * 8 * 4, teapot_nolid_raw->vertex_size);
	//shared_ptr<Model> toilet = make_shared<Model>(toilet_raw->vertices, toilet_raw->vertex_size * 8 * 4, toilet_raw->vertex_size);
	shared_ptr<ReflectionTexture> reflect_texture = make_shared<ReflectionTexture>(window_width,window_height);
	// [-.5,-.5,-.5] ~ [.5,.5,.5]
	shared_ptr<Model> cube = MeshBuilder::Cube();
	int frame_counter = 0;
	int frame_sample = 500;
	float fps = 0;
	float frame_start = 0;
	float delta = 0.0f;
	float delta_stamp = 0.0f;

	vector<vec3> axis = {
		{ 1,0,0},
		{-1,0,0},
		{0, 1,0},
		{0,-1,0},
		{0,0, 1},
		{0,0,-1},
	};
	vector<vec3> axis_up = {
		{0,-1,0},
		{0,-1,0},
		{0,0, 1},
		{0,0,-1},
		{0,-1,0},
		{0,-1,0},
	};


	//Main loop
	shared_ptr<Camera> light_camera = make_shared<Camera>();
	light_camera->fov = 90;
	light_camera->nearp = 1.0f;
	light_camera->farp = 100.0f;
	light_camera->windowResize(SHADOW_WIDTH, SHADOW_HEIGHT);
	shadow_program->use();
	shadow_program->setInt("texture1", 0);
	shadow_program->setInt("texture2", 1);
	shadow_program->setFloat("far_plane", light_camera->farp);

	shader_program->use();
	shader_program->setInt("texture1", 0);
	shader_program->setInt("texture2", 1);

	for (int l = 0; l <= point_lights.size(); ++l) {
		shader_program->setInt("depthmap[" +std::to_string(l)+ "]", 2 + l);
	}
	
	shader_program->setFloat("far_plane", light_camera->farp);

	auto updateProgram = [&]() {
		shader_program->use();
		shader_program->setMat4("view", camera->view);
		shader_program->setMat4("proj", camera->proj);
		shader_program->setVec3("view_position", camera->position);
		shader_program->setInt("showing_depth_map", showing_depth_map);

	};


	shared_ptr<DynamicCubeMap> dynamic_cube_map = make_shared<DynamicCubeMap>(1024,point_lights.size(), light_camera);
	dynamic_cube_map->shadow_program = shadow_program;
	dynamic_cube_map->depthFBO = depthFBO;
	dynamic_cube_map->depthmap = depthmap;

	physic_solver = PhysicsSolver(voxelizer);

	auto sphere_mesh = MeshBuilder::Sphere(10);
	for (int i = 0; i < 1; ++i) {

		shared_ptr<GameObject> moving_sphere = make_shared<GameObject>(teapot_nolid, texture_yellow);
		moving_sphere->position = vec3(-7.479, -3.464, 7.024);
		moving_sphere->velocity = vec3(0, 0, 0);
		moving_sphere->scale = vec3(0.5);
		moving_sphere->mass = 157.08;
		physic_solver.add_entity(moving_sphere);
	}

	shared_ptr<GameObject> light_sphere = make_shared<GameObject>(teapot_nolid, texture_blue);
	light_sphere->position = vec3(6, 8, 6);
	light_sphere->velocity = vec3(0, 0.1, 0);
	light_sphere->scale = vec3(0.5);
	light_sphere->mass = 157.08;
	light_sphere->visible = false;
	physic_solver.add_entity(light_sphere);

	shared_ptr<GameObject> moving_teapot = make_shared<GameObject>(teapot_nolid, texture_yellow);
	//shared_ptr<GameObject> moving_toilet = make_shared<GameObject>(toilet, toilet_texture);
	shared_ptr<GameObject> moving_cube = make_shared<GameObject>(cube, texture_yellow);

	shared_ptr<GameObject> moving_metal_cube = make_shared<GameObject>(cube, texture_yellow);

	shared_ptr<GameObject> big_water_tank = make_shared<GameObject>(cube, texture_white);
	float tank_size = 10.f;
	water_grid = make_shared<WaterGrid>(130, vec3(tank_size, 5, tank_size), texture_blue);
	water_grid->internal_object->position = vec3(0.0);
	water_grid->internal_object->visible = false;
	auto env_cam_pos = 0.5 * (water_grid->internal_object->scale - water_grid->internal_object->position);
	//env_cam_pos;
	//dynamic_cube_map->refract_model_position = 0.5 * (water_grid->internal_object->scale - water_grid->internal_object->position) - vec3(0,-5,0);


	reflect_texture->plane_o = water_grid->internal_object->position + vec3(0, 1, 0) * (water_grid->internal_object->scale.y);
	reflect_texture->plane_u = vec3(0, 0, 1) * (water_grid->internal_object->scale.z);
	reflect_texture->plane_v = vec3(1, 0, 0) * (water_grid->internal_object->scale.x);


	big_water_tank->penetrable = true;
	big_water_tank->mass = -1;
	big_water_tank->density = 997.0; //997kg/m^3
	big_water_tank->scale = vec3(10, 5, 10);
	big_water_tank->position = vec3(0.0);

	moving_cube->position = vec3(3,10,3);
	moving_cube->scale = vec3(1.0f,0.3f,1.0f);
	moving_cube->mass = 300;


	moving_metal_cube->mass = 2000;
	moving_metal_cube->position = vec3(-2, 5, -2);
	moving_metal_cube->scale = vec3(0.3f);

	moving_teapot->position = vec3(5,20,5);
	moving_teapot->scale = vec3(1);
	moving_teapot->mass = 1000;

	/*moving_toilet->scale = vec3(0.1);
	moving_toilet->mass = 500;
	moving_toilet->orientation = quat(0,0,0,1);
	moving_toilet->position = vec3(6, 10, 6);*/
	
	//physic_solver.add_entity(moving_toilet);
	physic_solver.add_entity(moving_metal_cube);
	physic_solver.add_entity(moving_cube);
	physic_solver.add_entity(water_grid->internal_object);
	physic_solver.add_entity(moving_teapot);

	for (int dx = 1; dx >= -1; --dx) {
		for (int dy = 1; dy >= -1; --dy) {
			if (dx == 0 || dy == 0) {
				if (dx == 0 && dy == 0) continue;
				shared_ptr<GameObject> wallN = make_shared<GameObject>(MeshBuilder::Cube(), warning_tape);
				wallN->mass = -1;
				wallN->scale = vec3(10,7,10);
				wallN->position = big_water_tank->position + vec3(5,5,5) + 10 * vec3(dx, 0, dy) + 0 * vec3(dx, 0, dy);
				//physic_solver.add_entity(wallN);
			}
		}
	}
	shared_ptr<GameObject> wallN = make_shared<GameObject>(cube, texture_white);
	wallN->mass = -1;
	wallN->scale = vec3(10,10,10);
	wallN->position = vec3(5, -5, 5);
	physic_solver.add_entity(wallN);

	shared_ptr<GameObject> wallR = make_shared<GameObject>(cube, texture_white);
	wallR->mass = -1;
	wallR->scale = vec3(5,5,5);
	wallR->position = vec3(-7.720,-6.470,7.087);
	physic_solver.add_entity(wallR);

	const float dt = 1/60.0;

	shader_program->use();
	shader_program->setVec3("solid_color",vec3(1));
	while (!glfwWindowShouldClose(window)) {
#pragma region fpsCounter
		frame_counter++;
		if (frame_counter >= frame_sample) {
			fps = frame_counter / (glfwGetTime() - frame_start);
			frame_counter = 0;
			frame_start = glfwGetTime();
		}
#pragma endregion
		//Adjust camera speed \w to fps
		delta = glfwGetTime() - delta_stamp;
		delta_stamp = glfwGetTime();
		updateWorld(window,delta);

		physic_solver.update(dt,water_grid, pause_world);
		if (!pause_world) {
			if (cur_time != time_front) time_front = cur_time;
			obj_states[time_front].clear();
			for (auto c : physic_solver.entity_list) {
				obj_states[time_front].push_back(*c);
			}
			++time_front;
			cur_time = time_front;
			mx_time_front = std::max(time_front, mx_time_front);
			water_grid->update();
			voxelizer->voxelize(water_grid->internal_object);
		}
		else {
			if (cur_time != time_front) {
				time_front = cur_time;

				physic_solver.entity_list.clear();
				for (auto c : obj_states[cur_time]) physic_solver.entity_list.push_back(make_shared<GameObject>(c));
			}
		}
		auto getMouseSelect = [&]() {
			auto o = world_mouse;
			auto ray_direct = far_world_mouse - world_mouse;
			ray ray_t(o,uni(ray_direct));
			interval ri(0, 100); // far plane
			shared_ptr<GameObject> obj;
			if (camera_control) return obj;
			return obj;
			for (auto& a : physic_solver.entity_list) {
				if (a->penetrable) continue;
				a->update_aabb();
				a->selected = false;
				if (a->bounding_box->hit(ray_t, ri)) {
					voxelizer->voxelize(a);
					if (physic_solver.collision_detector->intersect_with_ray(a,ray_t,ri)) {
						obj = a;
					}

				}
			}
			if (obj) obj->selected = true;
			return obj;
		};
		if (!valid_dragging) {
			selected = getMouseSelect();
		}
		auto draw_scence = [&](shared_ptr<ShaderProgram> shader_program) {
			shader_program->use();
			point_lights[2].position[0] = light_sphere->position.x;
			point_lights[2].position[1] = light_sphere->position.y;
			point_lights[2].position[2] = light_sphere->position.z;

#pragma region LightParameterPass
			for (int i = 0; i < point_lights.size(); ++i) {
				shader_program->setVec3("point_lights[" + std::to_string(i) + "].position", point_lights[i].position);
				shader_program->setVec3("point_lights[" + std::to_string(i) + "].ambient", point_lights[i].ambient);
				shader_program->setVec3("point_lights[" + std::to_string(i) + "].diffuse", point_lights[i].diffuse);
				shader_program->setVec3("point_lights[" + std::to_string(i) + "].specular", point_lights[i].specular);
				shader_program->setVec3("point_lights[" + std::to_string(i) + "].attenuation", point_lights[i].attenuation);
				shader_program->setInt("point_lights[" + std::to_string(i) + "].enable", point_lights[i].enable);
			}
#pragma endregion

			//Engraving texture
			glActiveTexture(GL_TEXTURE1);
			texture_white->bind();

			for (int l = 0; l < point_lights.size(); ++l) {
				if (point_lights[l].enable && l != 2) {
					shader_program->setInt("isLight", 1 + l);
					shader_program->setMat4("model", mat4::trans(point_lights[l].position));
					cube->draw();
				}
			}
			if (point_lights[2].enable) {

				shader_program->setInt("isLight", 1 + 2);
				shader_program->setMat4("model", light_sphere->localToWorld());
				if (light_sphere->selected) {
					visualizer.draw_glow(light_sphere, vec4(1, 0, 0, 0.8));
					shader_program->use();
				}
				light_sphere->model->draw();
			}

			shader_program->setInt("isLight", 0);

			shader_program->setMat4("textureMat", mat4::scale(4));
			shader_program->setMat4("model", mat4::Rx(0.1 * glfwGetTime()));
			shader_program->setMat4("model", mat4::trans(world_mouse) * mat4::scale(0.01));
			shader_program->use();

			glActiveTexture(GL_TEXTURE0);
			texture_blue->bind();
			

			for (auto& a : physic_solver.entity_list) {
				if (!a->visible) continue;
				if (a->selected) {
					visualizer.draw_glow(a,vec4(1,0,0,0.8));
					shader_program->use();
				}
				a->draw(shader_program);
			}

			shader_program->setFloat("opacity", 1.0f);
			water_grid->internal_object->draw(shader_program);
			shader_program->setFloat("opacity", 1.0f);
		};


		for (int l = 0; l < point_lights.size(); ++l) {
			shadow_program->use();
			light_camera->position = point_lights[l].position;
			for (int i = 0; i < 6; ++i) {
				light_camera->vup = axis_up[i];
				light_camera->lookAt(axis[i] + point_lights[l].position);
				auto mat = light_camera->getMatrix();
				shadow_program->setMat4("lightSpaceMatrices[" + std::to_string(i) + "]", light_camera->getMatrix());
			}
			shadow_program->setInt("current_light", l);
			glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
			glBindFramebuffer(GL_FRAMEBUFFER, depthFBO[l]);

			glClear(GL_DEPTH_BUFFER_BIT);
			draw_scence(shadow_program);
		}

		dynamic_cube_map->drawBuffer(draw_scence);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		auto draw_half_world = [&]() {
			updateProgram();
			draw_scence(shader_program);
		};

		auto draw_world = [&]() {
			draw_half_world();

			/*glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, dynamic_cube_map->environMap);
			glActiveTexture(GL_TEXTURE1);
			water_grid->internal_object->texture->bind();
			glBindTexture(GL_TEXTURE_CUBE_MAP, dynamic_cube_map->environMap);
			refract_program->use();
			refract_program->setMat4("view", camera->view);
			refract_program->setMat4("proj", camera->proj);
			refract_program->setVec3("camera_position", camera->position);
			refract_program->setInt("reflection", 0);
			refract_program->setMat4("textureMat", mat4::identity());
			refract_program->setMat4("model", mat4::trans(dynamic_cube_map->refract_model_position));*/
			//teapot->draw();
			
			shader_program->use();

			//reflect_texture->render(camera,draw_half_world, window_width, window_height);

			glViewport(0, 0, window_width, window_height);
			//for (auto& a : physic_solver.entity_list) visualizer.draw_voxel(a);
			//for (auto& a : physic_solver.entity_list) visualizer.draw_voxel_collision(a);
			if (view_aabb) for (auto& a : physic_solver.entity_list) visualizer.draw_aabb(*(a->bounding_box));
			if(view_points) for (auto& a : physic_solver.entity_list) visualizer.draw_point(a->getWorldGravityCenter(), { 0,1,1,1 });
			if (view_trajectory) {
				// gravity center = toWorld() * origin = position, taken from the recorded frame history
				for (size_t idx = 0; idx < physic_solver.entity_list.size(); ++idx) {
					auto& a = physic_solver.entity_list[idx];
					if (a->hasInifiniteMass() || a->penetrable) continue;
					vector<vec3> pts;
					for (int t = std::max(0, cur_time - trajectory_length); t < cur_time; ++t)
						if (idx < obj_states[t].size()) pts.push_back(obj_states[t][idx].position);
					pts.push_back(a->getWorldGravityCenter());
					visualizer.draw_polyline(pts, { 1,0.5,0,1 });
				}
			}
			for (auto& a : physic_solver.collision_detector->collisions) {
				//if (a.inwater) continue;
				if (view_collision) visualizer.draw_voxel_collision(a.draw_onB ? a.B : a.A);
				if (!view_collision_points) continue;
				//visualizer.draw_point(a.point, { 1,1,0,1 });
				float peneA = abs(a.A->localToWorld() * a.data.Pa - a.A->localToWorld() * a.data.Sa);
				float peneB = abs(a.B->localToWorld() * a.data.Pb - a.B->localToWorld() * a.data.Sb);
				visualizer.draw_vector(a.data.Na * peneA, a.A->localToWorld() * a.data.Pa, { 0,0,1,1 });
				visualizer.draw_point(a.A->localToWorld() * a.data.Pa, { 1,1,0,1 });

				visualizer.draw_vector(a.data.Nb * peneB, a.B->localToWorld() * a.data.Pb, {0,1,1,1});
				visualizer.draw_point(a.B->localToWorld() * a.data.Pb, { 0,1,0,1 });
			}
			
			if(selected != nullptr)
			visualizer.draw_distance(selected, distance_visualization_slice);

			if (selected_forAdjustment != nullptr)
				visualizer.draw_distance(selected_forAdjustment, distance_visualization_slice);
			//visualizer.draw_point(mmpos, { 1,1,1,0.4 });
		/*	visualizer.draw_point(world_mouse, {1,1,1,0.4});
			visualizer.draw_vector(uni(far_world_mouse - world_mouse), world_mouse, {1,1,1,0.4});*/
		};
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		if (split_window) {
			auto old_pos = camera->position;
			vector<float> dx{ 0,d_width };
			vector<float> dy{ 0,d_height };
			for (int i = 0; i < 2; ++i) {
				for (int j = 0; j < 2; ++j) {
					glViewport(dx[i], dy[j], d_width, d_height);
					if (i == j && j == 1) {
						camera->position = old_pos;
						camera->windowResize(d_width, d_height);
						camera->updateView();
						draw_world();
					}
					else {
						camera->position = axis[(i * 2 + j) * 2] * 10.5 + (vec3(1) - axis[(i * 2 + j) * 2]);
						camera->lookAt(vec3(10.5) + (vec3(1) - axis[(i * 2 + j) * 2]));
						camera->make_ortho(d_width, d_height, 10);
						draw_world();
					}
				}
			}
		}
		else {
			glViewport(0, 0, window_width, window_height);
			draw_world();
		}

		render_ui(fps);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}