
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
int ball_batch = 1;
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
	ImGui::SliderInt("ball batch",&ball_batch, 1, 1000);
	if (ImGui::Button("Add ball to Scence")) {
		std::cout << "added balls : " << ball_batch << "\n";
		add_ball(ball_batch);
	}
	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
float cube_vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
											 		   
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,
											 		   
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
											  		   
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
											  		   
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
											  		   
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f
};											 

const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

const unsigned int ENV_WIDTH = 1024, ENV_HEIGHT = 1024;
int window_width = WINDOW_WIDTH, window_height = WINDOW_HEIGHT;

const unsigned int REGULAR_DIVISION = 8;
const int regular_div = 21 / (REGULAR_DIVISION - 1);
vector<shared_ptr<GameObject>> regular_grid[REGULAR_DIVISION + 1][REGULAR_DIVISION + 1][REGULAR_DIVISION + 1];

signed main() {

	camera = make_shared<Camera>();
	camera->position = vec3(5, 5, 5);
	camera->lookAt({ 0,0,0 });
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
	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		camera->windowResize(width, height);
		window_width = width;
		window_height = height;
	});

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
	glEnable(GL_DEPTH_TEST);

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
	shared_ptr<Model> cube = make_shared<Model>(cube_vertices, sizeof(cube_vertices), 36);
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

	shared_ptr<MeshBuilder> mesh_builder = make_shared<MeshBuilder>();
	shared_ptr<Model> sphere_mesh = mesh_builder->buildSphere(10);
	shared_ptr<Model> plane_mesh = mesh_builder->buildPlane(1,vec3(0,0,1),vec3(1,0,0),vec3(0.0f));

	vector<shared_ptr<GameObject>> entity_list;
	vector<shared_ptr<Sphere>> spheres;
	vector<shared_ptr<Plane>> floor;
	vector<shared_ptr<Plane>> walls;

	for (int i = 0; i < 21; ++i) {
		for (int j = 0; j < 21; ++j) {
			shared_ptr<Plane> plane = make_shared<Plane>(vec3(i,0,j));
			plane->model = plane_mesh;
			plane->texture = (i + j) % 2 ? texture_white : texture_blue;
			floor.push_back(plane);
		}
	}
#pragma region prepareWalls


	auto plane = make_shared<Plane>(vec3(0,21,0), vec3(21) ,vec3(pi / 2,0,0));
	plane->texture = texture_yellow;
	plane->model = plane_mesh;
	walls.push_back(plane);
	plane = make_shared<Plane>(vec3(0, 21, 0), vec3(21), vec3(0, 0, -pi/2));
	plane->texture = texture_green;
	plane->model = plane_mesh;
	walls.push_back(plane);
	plane = make_shared<Plane>(vec3(21, 0, 0), vec3(21), vec3(0, 0, pi / 2));
	plane->texture = texture_green;
	plane->model = plane_mesh;
	walls.push_back(plane);
	plane = make_shared<Plane>(vec3(0, 0, 21), vec3(21), vec3(-pi/2, 0, 0));
	plane->texture = texture_yellow;
	plane->model = plane_mesh;
	walls.push_back(plane);
#pragma endregion

	add_ball = [&](int ball_count) {
		for (int i = 0; i < ball_count; ++i) {
			auto tmp = make_shared<Sphere>(0.5, vec3(10, 10, 10));
			tmp->model = sphere_mesh;
			tmp->texture = texture_white;
			tmp->velocity.x = random_float() - 0.5f;
			tmp->velocity.y = random_float() - 0.5f;
			tmp->velocity.z = random_float() - 0.5f;

			tmp->position.x = (random_float() - 0.5f) * 10.f + 10.f;
			tmp->position.y = (random_float() - 0.5f) * 10.f + 10.f;
			tmp->position.z = (random_float() - 0.5f) * 10.f + 10.f;

			tmp->velocity *= 100;
			spheres.push_back(tmp);
			entity_list.push_back(tmp);
		}
	};
	auto resolve_collision = [&](auto &a, auto &b) {
		if (a->type == b->type && a->type == SPHERE) {
			auto A = std::static_pointer_cast<Sphere>(a);
			auto B = std::static_pointer_cast<Sphere>(b);
			auto w = A->position - B->position;
			auto d = abs(w);
			auto n = -w;
			auto x = (A->radius + B->radius) - d;
			if (x >= 0) {
				a->texture = texture_white;
				auto vel = a->velocity;
				a->forces += x * n; // reflect
			}
		}

		if (a->type == SPHERE && b->type == PLANE) {
			auto sphere = std::static_pointer_cast<Sphere>(a);
			auto plane = std::static_pointer_cast<Plane>(b);
			auto [u, v] = plane->transformedPlane();
			auto n = uni(u ^ v);
			auto d = (sphere->position - plane->position) * n;
			if (d > sphere->radius) return;
			auto p = (sphere->position - (d * n)) - plane->position;
			//suppose u perpendicular to v
			auto up = (p * uni(u)) / abs(u);
			auto vp = (p * uni(v)) / abs(v);

			//Collided
			if (0 <= up <= 1 && 0 <= vp && vp <= 1) {
				a->texture = b->texture;
				auto vel = a->velocity;
				//a->velocity = vel - 2 * (n * vel) * n; // reflect
				a->forces += -2 * (n * vel) * n;
			}
		}
	};
	auto collision_solve_naive = [&]() {
		for (auto& a : entity_list) {
			for (auto& b : entity_list) {
				if (a == b) continue;
				resolve_collision(a, b);
			}
		}
	};

	auto collision_solve_regular = [&]() {
		for (auto& a : entity_list) {
			if (a->type == PLANE) continue;
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

						hitlist.insert(hitlist.end(), regular_grid[x][y][z].begin(), regular_grid[x][y][z].end());
					}
				}
			}
			std::sort(hitlist.begin(), hitlist.end());
			hitlist.erase(std::unique(hitlist.begin(), hitlist.end()),hitlist.end());
			for (auto& b : hitlist) {
				if (a == b) continue;
				// a react to b
				resolve_collision(a, b);
			}
		}
		for (auto& c : regular_grid)
			for (auto& b : c)
				for (auto& d : b) {
					d.clear();
				}
	};

	const float dt = 0.001;
	auto update_positions = [&]() {
		for (auto& a : entity_list) {
			auto dv = uni(a->velocity + a->forces) * abs(a->velocity); // conservation of momentum
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
	for (auto& a : floor) entity_list.push_back(a);
	for (auto& a : spheres) entity_list.push_back(a);
	for (auto& a : walls) entity_list.push_back(a);


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
		//collision_solve_naive();
		collision_solve_regular();

		auto draw_scence1 = [&](shared_ptr<ShaderProgram> shader_program) {
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

			shader_program->setInt("engraved", 1);
			glActiveTexture(GL_TEXTURE0);
			texture_white->bind();
			glActiveTexture(GL_TEXTURE1);
			engraver->bind();
			cube->draw();

			for (int i = 0; i < 5; ++i) {
				if (i == 0) {
					shader_program->setMat4("textureMat", mat4::scale(32));
					shader_program->setInt("engraved", 1);
				}
				else {
					shader_program->setMat4("textureMat", mat4::trans(vec3(1, 0, 0) * glfwGetTime() * 0.1));
					shader_program->setInt("engraved", 0);
				}

				shader_program->setMat4("model", mat4::trans(vec3(-5 + i - 2 + cos(glfwGetTime() * ((i + 1) / 5.0)), i - 2 + cos(glfwGetTime() * ((i + 1) / 5.0)), -5 + i - 2 + sin(glfwGetTime() * ((i + 1) / 5.0)))) \
					* mat4::scale(0.1) * mat4::Rx(glfwGetTime() * 0.1 * (i&1 ? 1 : -1)) * mat4::Ry(glfwGetTime() * 0.1 * (i & 1 ? -1 : 1)) * mat4::Rz(glfwGetTime() * 0.1 * (i & 1 ? 1 : -1)));
				glActiveTexture(GL_TEXTURE0);
				texture_white->bind();
				if (i == 2) warp_tape->bind();
				else if (i == 3) wood_floor->bind();
				else if (i == 4) warning_tape->bind();
				teapot->draw();
			}

			shader_program->setInt("engraved", 1);
			shader_program->setMat4("model", mat4::trans(vec3(1, 1, 1)));
			shader_program->setMat4("textureMat", mat4::trans(vec3(1, 0, 0) * glfwGetTime() * 0.1));
			glActiveTexture(GL_TEXTURE0);
			warning_tape->bind();
			glActiveTexture(GL_TEXTURE1);
			warning_tape->bind();
			cube->draw();


			shader_program->setInt("engraved", 0);

			shader_program->setMat4("model", mat4::trans(vec3(5, -3.2, 5)) * mat4::scale(0.1));
			glActiveTexture(GL_TEXTURE0);
			texture_white->bind();
			teapot->draw();

			shader_program->setMat4("model", mat4::trans(vec3(2, 2, 1)) * mat4::Ry(0.5) * mat4::scale(0.1));
			glActiveTexture(GL_TEXTURE0);
			warp_tape->bind();
			teapot->draw();

			shader_program->setMat4("textureMat", mat4::scale(32));
			shader_program->setMat4("model", mat4::trans(vec3(0, -5, 0)) * mat4::scale(vec3(10)));
			glActiveTexture(GL_TEXTURE0);
			wood_floor->bind();
			teapot->draw();
		};
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

			glActiveTexture(GL_TEXTURE0);
			texture_blue->bind();
			

			for (auto& plane : floor) plane->draw(shader_program);
			for (auto& sphere : spheres) sphere->draw(shader_program);
			for (auto& plane : walls) plane->draw(shader_program);
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
		glViewport(0, 0, window_width, window_height);

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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