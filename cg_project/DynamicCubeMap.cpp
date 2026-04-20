#include "DynamicCubeMap.hpp"

DynamicCubeMap::DynamicCubeMap(int _size,int light_count, shared_ptr<Camera> light_camera) : size(_size), light_count(light_count), light_camera(light_camera){
	
	env_program = make_shared<ShaderProgram>
		(vector<shared_ptr<Shader>>{
			make_shared<Shader>("env.vert", GL_VERTEX_SHADER),
			make_shared<Shader>("env.frag", GL_FRAGMENT_SHADER),
			make_shared<Shader>("env.geom", GL_GEOMETRY_SHADER)
	});

	env_camera = make_shared<Camera>();
	env_camera->fov = 90;
	env_camera->nearp = 1.0f;
	env_camera->farp = 100.0f;
	env_camera->windowResize(size, size);

	env_program->use();
	env_program->setVec3("material.ambient", vec3(.2));
	env_program->setVec3("material.diffuse", vec3(.8));
	env_program->setVec3("material.specular", vec3(1));
	env_program->setFloat("material.shininess", 32);

	env_program->setInt("texture1", 0);
	env_program->setInt("texture2", 1);

	for (int l = 0; l <= light_count; ++l) {
		env_program->setInt("depthmap[" + std::to_string(l) + "]", 2 + l);
	}
	env_program->setFloat("far_plane", light_camera->farp);


	glGenTextures(1, &environMap);
	glGenFramebuffers(1, &environFBO);
	glBindTexture(GL_TEXTURE_CUBE_MAP, environMap);
	for (int i = 0; i < 6; ++i) {
		GLenum face = GL_TEXTURE_CUBE_MAP_POSITIVE_X + i;
		glTexImage2D(face, 0, GL_RGB, size, size, 0, GL_RGB, GL_FLOAT, 0);
	}
	// Render the scence in a single pass 
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	glBindFramebuffer(GL_FRAMEBUFFER, environFBO);

	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, environMap, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void DynamicCubeMap::drawBuffer(std::function<void(shared_ptr<ShaderProgram>)> draw_scence) {

	shadow_program->use();
	env_camera->position = refract_model_position;
	for (int i = 0; i < 6; ++i) {
		env_camera->vup = axis_up[i];
		env_camera->lookAt(axis[i] + refract_model_position);
		shadow_program->setMat4("lightSpaceMatrices[" + std::to_string(i) + "]", env_camera->getMatrix());
	}
	shadow_program->setInt("current_light", -1);
	glViewport(0, 0, shadow_width, shadow_height);
	glBindFramebuffer(GL_FRAMEBUFFER, depthFBO[light_count]);

	glClear(GL_DEPTH_BUFFER_BIT);
	draw_scence(shadow_program);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	for (int l = 0; l <= light_count; ++l) {
		glActiveTexture(GL_TEXTURE2 + l);
		glBindTexture(GL_TEXTURE_CUBE_MAP, depthmap[l]);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, environFBO);
	glViewport(0, 0, size, size);

	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	env_program->use();
	env_camera->position = refract_model_position;
	for (int i = 0; i < 6; ++i) {
		env_camera->vup = axis_up[i];
		env_camera->lookAt(axis[i] + refract_model_position);
		env_program->setMat4("spaceMatrices[" + std::to_string(i) + "]", env_camera->getMatrix());
	}
	env_program->setVec3("view_position", refract_model_position);
	draw_scence(env_program);
}