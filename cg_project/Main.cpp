
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
#include "VoxelHelper.hpp"

shared_ptr<Camera> camera;

bool camera_control = false;
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
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_E && action == GLFW_PRESS){
		camera_control = !camera_control;
		glfwSetInputMode(window, GLFW_CURSOR, camera_control ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		exit(0);
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
	camera->updateView();

	float R = 10.0f;
	point_lights[2].position[0] = R * cos(glfwGetTime() * 0.1) + R;
	point_lights[2].position[2] = R * sin(glfwGetTime() * 0.1) + R;
}
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	static double last_x = xpos, last_y = ypos;
	double dx = xpos - last_x, dy = ypos - last_y;
	last_x = xpos, last_y = ypos;
	if(camera_control)
	camera->mouseMove(dx, -dy);
}


bool showing_depth_map = 0;
bool enable_acc_structure = 1;
int ball_batch = 1;
float ball_speed_amplifiler = 1.0;
bool split_window = false;
void render_ui(float fps) {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Light Configurator");
	ImGui::Text(("fps:" + std::to_string(fps)).c_str());
	ImGui::Checkbox("showing depth map", &showing_depth_map);
	ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
	for (int i = 0; i < point_lights.size(); ++i) {
		if (ImGui::TreeNodeEx(("Point Light #" + std::to_string(i)).c_str(), flag)) {
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
	ImGui::SliderInt("Ball Batch",&ball_batch, 1, 1000);
	//ImGui::Checkbox("Accelerated Structure",&enable_acc_structure);
	ImGui::SliderFloat("Ball Initial Speed", &ball_speed_amplifiler, 0.0, 20.0);

	ImGui::Checkbox("Split View", &split_window);
	if (ImGui::Button("Add ball to Scence")) {
		std::cout << "added balls : " << ball_batch << "\n";
		add_ball(ball_batch);
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
											 

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

const unsigned int ENV_WIDTH = 1024, ENV_HEIGHT = 1024;
int window_width = WINDOW_WIDTH, window_height = WINDOW_HEIGHT;

const unsigned int REGULAR_DIVISION = 8;
const int regular_div = 21 / (REGULAR_DIVISION - 1);
vector<shared_ptr<GameObject>> regular_grid[REGULAR_DIVISION + 1][REGULAR_DIVISION + 1][REGULAR_DIVISION + 1];
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
	camera->position = vec3(10, 10, 10);
	camera->lookAt({ 10,10,10 });
	camera->yx = pi / 2;
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
	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity,
		GLsizei length, const GLchar* message, const void* userParam) {
			fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
				(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
				type, severity, message);
		}, 0);

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

	shared_ptr<ShaderProgram> voxelizer_program = make_shared<ShaderProgram>
		(vector<shared_ptr<Shader>>{
		make_shared<Shader>("voxelizer.vert", GL_VERTEX_SHADER),
			make_shared<Shader>("voxelizer.frag", GL_FRAGMENT_SHADER)
	});


	shared_ptr<ShaderProgram> collision_program = make_shared<ShaderProgram>
		(vector<shared_ptr<Shader>>{
			make_shared<Shader>("collision.comp", GL_COMPUTE_SHADER)
	});


	shared_ptr<ShaderProgram> voxel_visualizer_program = make_shared<ShaderProgram>
		(vector<shared_ptr<Shader>>{
		make_shared<Shader>("voxelvisualizer.vert", GL_VERTEX_SHADER),
			make_shared<Shader>("voxelvisualizer.frag", GL_FRAGMENT_SHADER)
	});


	shared_ptr<ShaderProgram> aabb_visualizer_program = make_shared<ShaderProgram>
		(vector<shared_ptr<Shader>>{
		make_shared<Shader>("aabbvisualizer.vert", GL_VERTEX_SHADER),
			make_shared<Shader>("aabbvisualizer.frag", GL_FRAGMENT_SHADER)
	});




	shared_ptr<Texture> texture_white = make_shared<Texture>();
	shared_ptr<Texture> texture_blue = make_shared<Texture>(0xff0000);

	shared_ptr<Texture> texture_yellow = make_shared<Texture>(0x00ffff);
	shared_ptr<Texture> texture_green = make_shared<Texture>(0x00ff00);

	shared_ptr<Texture> warning_tape = make_shared<Texture>("warningTape.png");
	shared_ptr<Texture> warp_tape = make_shared<Texture>("warp.jpg");
	shared_ptr<Texture> wood_floor = make_shared<Texture>("wood.png");

	shared_ptr<Texture> engraver = make_shared<Texture>(10, 10);

	// Setup lights
	point_lights.push_back({
		{10.5,21.5,10.5}, //position
		{0.5,0.5,0.5}, //ambient
		{1,1,1}, //diffuse
		{.5,.5,.5}, //specular
		{0,.2,1}, //attenuation
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

	
	shader_program->use();

	shared_ptr<ModelLoader> teapot_raw = make_shared<ModelLoader>("utah_teapot.obj");
	shared_ptr<Model> teapot = make_shared<Model>(teapot_raw->vertices, teapot_raw->vertex_size * 8 * 4, teapot_raw->vertex_size);
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

	shared_ptr<GameObject> moving_sphere = make_shared<GameObject>(MeshBuilder::Sphere(10), texture_yellow);
	shared_ptr<GameObject> moving_teapot = make_shared<GameObject>(teapot, texture_yellow);
	moving_teapot->position = vec3(12, 10, 10);
	moving_teapot->scale = vec3(0.5f);

	moving_sphere->position = vec3(10, 10, 10);

	vector<shared_ptr<GameObject>> entity_list;
	vector<shared_ptr<GameObject>> draw_list;


	auto resolve_collision = [&](shared_ptr<GameObject> a, shared_ptr<GameObject> b) {
		a->voxelize(voxelizer_program);
		b->voxelize(voxelizer_program);

		glBindImageTexture(0, a->voxelTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);
		glBindImageTexture(1, b->voxelTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

		glBindImageTexture(2, a->collisionVisualizeTexture, 0, GL_TRUE, 0, GL_READ_WRITE, GL_R32UI);

		collision_program->use();
		collision_program->setInt("voxelInfoA.dimx", a->voxel_info.dim_x);
		collision_program->setInt("voxelInfoA.dimy", a->voxel_info.dim_y);
		collision_program->setInt("voxelInfoA.dimz", a->voxel_info.dim_z);

		collision_program->setInt("voxelInfoB.dimx", b->voxel_info.dim_x);
		collision_program->setInt("voxelInfoB.dimy", b->voxel_info.dim_y);
		collision_program->setInt("voxelInfoB.dimz", b->voxel_info.dim_z);

		collision_program->setInt("voxelInfoA.corner_x", a->voxel_info.corner_x);
		collision_program->setInt("voxelInfoA.corner_y", a->voxel_info.corner_y);
		collision_program->setInt("voxelInfoA.corner_z", a->voxel_info.corner_z);

		collision_program->setInt("voxelInfoB.corner_x", b->voxel_info.corner_x);
		collision_program->setInt("voxelInfoB.corner_y", b->voxel_info.corner_y);
		collision_program->setInt("voxelInfoB.corner_z", b->voxel_info.corner_z);

		glDispatchCompute((a->voxel_info.dim_x + 7) / 8, (a->voxel_info.dim_y + 7) / 8, 1);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	};
	auto collision_solve_regular = [&]() {
		for (auto& a : entity_list) {
			vector<shared_ptr<GameObject>> hitlist;

			auto& bb = a->bounding_box;
			int mnx = bb->x.min / regular_div;
			int mxx = bb->x.max / regular_div;
					
			int mny = bb->y.min / regular_div;
			int mxy = bb->y.max / regular_div;
						
			int mnz = bb->z.min / regular_div;
			int mxz = bb->z.max / regular_div;

	 		for (int x = mnx; x <= mxx; ++x) {
	 			for (int y = mny; y <= mxy; ++y) {
	 				for (int z = mnz; z <= mxz; ++z) {
	 					if (x >= REGULAR_DIVISION || x < 0) continue;
	 					if (y >= REGULAR_DIVISION || y < 0) continue;
	 					if (z >= REGULAR_DIVISION || z < 0) continue;
						for (auto& b : regular_grid[x][y][z]) {
							if (a == b) continue; // hit self
							if (a > b) continue; // resolved
							if(a->bounding_box->hit(*b->bounding_box)){
								hitlist.push_back(b);
								//fast check by using AABB
							}
						}
	 				}
	 			}
	 		}
	 		std::sort(hitlist.begin(), hitlist.end());
	 		hitlist.erase(std::unique(hitlist.begin(), hitlist.end()),hitlist.end());
	 		for (auto& b : hitlist) {
				// futher check by using voxelization result
	 			resolve_collision(a, b);
	 		}
	 	}
	 };

	const float dt = 0.001;
	auto update_positions = [&]() {

		for (auto& c : regular_grid)
			for (auto& b : c)
				for (auto& d : b) {
					d.clear();
				}

		for (auto& a : entity_list) {
			//a->forces += vec3(0, -1, 0);
			auto dv = uni(a->velocity + a->forces) * abs(a->velocity); // conservation of momentum
			//auto dv = a->velocity + a->forces;
			if (abs(a->velocity + a->forces) < 1e-4) {
				dv = 0.0f;
			}
			a->velocity = dv;
			a->position += dv * dt;
			a->forces = vec3(0.0);
			a->update_aabb();
			
			auto& bb = a->bounding_box;
			int mnx = bb->x.min / regular_div;
			int mxx = bb->x.max / regular_div;
						
			int mny = bb->y.min / regular_div;
			int mxy = bb->y.max / regular_div;
						
			int mnz = bb->z.min / regular_div;
			int mxz = bb->z.max / regular_div;

			for (int x = mnx; x <= mxx; ++x) {
				for (int y = mny; y <= mxy; ++y) {
					for (int z = mnz; z <= mxz; ++z) {
						if (x >= REGULAR_DIVISION || x < 0) continue;
						if (y >= REGULAR_DIVISION || y < 0) continue;
						if (z >= REGULAR_DIVISION || z < 0) continue;

						regular_grid[x][y][z].push_back(a);
					}
				}
			}
		}
	};
	entity_list.push_back(moving_sphere);
	entity_list.push_back(moving_teapot);
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

		update_positions();
		collision_solve_regular();
		

		auto draw_scence = [&](shared_ptr<ShaderProgram> shader_program) {
			shader_program->use();
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
				if (point_lights[l].enable) {
					shader_program->setInt("isLight", 1 + l);
					shader_program->setMat4("model", mat4::trans(point_lights[l].position));
					cube->draw();
				}
			}

			shader_program->setInt("isLight", 0);

			shader_program->setMat4("textureMat", mat4::scale(4));
			shader_program->setMat4("model", mat4::Rx(0.1 * glfwGetTime()));
			shader_program->setMat4("model", mat4::trans(vec3(2, 2, 1)));
			shader_program->use();

			glActiveTexture(GL_TEXTURE0);
			texture_blue->bind();
			
			for (auto& a : draw_list) a->draw(shader_program);
			
			moving_sphere->draw(shader_program);
			moving_teapot->draw(shader_program);

			moving_sphere->update_aabb();
			
			//moving_sphere->voxelize(voxelizer_program);
			glViewport(0, 0, window_width, window_height);
			glDisable(GL_DEPTH_TEST);
			//VoxelHelper::draw_voxel(moving_sphere, voxel_visualizer_program, camera, cube);
			VoxelHelper::draw_voxel_collision(moving_sphere, voxel_visualizer_program, camera, cube);

			moving_teapot->update_aabb();
			//VoxelHelper::draw_aabb(*moving_teapot->bounding_box, aabb_visualizer_program, camera, cube);
			//voxel_sphere->update_aabb();
			//voxel_sphere->draw(shader_program);
			//moving_teapot->voxelize(voxelizer_program);
			glViewport(0, 0, window_width, window_height);
			//VoxelHelper::draw_voxel(moving_teapot, voxel_visualizer_program, camera, cube);
			VoxelHelper::draw_voxel_collision(moving_teapot, voxel_visualizer_program,camera, cube);
			glEnable(GL_DEPTH_TEST);
		};


		for (int l = 0; l < point_lights.size(); ++l) {
			shadow_program->use();
			light_camera->position = point_lights[l].position;
			for (int i = 0; i < 6; ++i) {
				light_camera->vup = axis_up[i];
				light_camera->lookAt(axis[i] + point_lights[l].position);
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

		auto draw_world = [&]() {
			updateProgram();
			draw_scence(shader_program);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_CUBE_MAP, dynamic_cube_map->environMap);
			refract_program->use();
			refract_program->setMat4("view", camera->view);
			refract_program->setMat4("proj", camera->proj);
			refract_program->setVec3("camera_position", camera->position);
			refract_program->setInt("reflection", 1);
			refract_program->setMat4("model", mat4::trans(dynamic_cube_map->refract_model_position));
			teapot->draw();
		
			shader_program->use();

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