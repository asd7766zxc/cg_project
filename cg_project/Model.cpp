#include "Model.hpp"
const int MXN = 1e6 + 5;
float positions[MXN];
float norms[MXN];
void Model::initializeBuffers() {
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glGenBuffers(1, &triangles);
	glGenBuffers(1, &normals);
}
Model::Model() {
	initializeBuffers();
}
void Model::bind_buffer() {
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangles);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, normals);
}
Model::Model(float *vertices, int size, int vertex_count) : vertex_count(vertex_count) {
	initializeBuffers();
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat) * 3));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(sizeof(GLfloat) * 6));
	glEnableVertexAttribArray(2);
	float mx = 0.0f;
	float mn = 0.0f;
	for(int i = 0; i < vertex_count; ++i) {
		float x = vertices[i * 8 + 0];
		float y = vertices[i * 8 + 1];
		float z = vertices[i * 8 + 2];
		
		mx = std::fmax(x, mx);
		mx = std::fmax(y, mx);
		mx = std::fmax(z, mx);

		mn = std::fmin(x, mn);
		mn = std::fmin(y, mn);
		mn = std::fmin(z, mn);

		float nx = vertices[i * 8 + 3];
		float ny = vertices[i * 8 + 4];
		float nz = vertices[i * 8 + 5];
		bounding_box.x.adjust(x);
		bounding_box.y.adjust(y);
		bounding_box.z.adjust(z);

		positions[i * 3 + 0] = x;
		positions[i * 3 + 1] = y;
		positions[i * 3 + 2] = z;

		// make mesh aabb
	}
	for (int i = 0; i < vertex_count; i += 9) {
		auto v0 = vec3(&positions[(i + 0) * 3]);
		auto v1 = vec3(&positions[(i + 1) * 3]);
		auto v2 = vec3(&positions[(i + 2) * 3]);

		auto norm = uni((v1 - v0) ^ (v2 - v1));
		
		float nx = vertices[i * 8 + 3];
		float ny = vertices[i * 8 + 4];
		float nz = vertices[i * 8 + 5];
		vec3 ori_norm = vec3(nx, ny, nz);
		if (ori_norm * norm < 0) {
			norm = -norm;
		}

		norms[(i / 9) * 3 + 0] = nx;
		norms[(i / 9) * 3 + 1] = ny;
		norms[(i / 9) * 3 + 2] = nz;
	}

	for (int i = 0; i < vertex_count; ++i) {
		
		// x -> x - mn -> (x - mn) / (mx - mn)
		positions[i * 3 + 0] = (positions[i * 3 + 0] - mn) / (mx - mn); //[mn,mx] -> [0,mx-mn] -> 
		positions[i * 3 + 1] = (positions[i * 3 + 1] - mn) / (mx - mn); 
		positions[i * 3 + 2] = (positions[i * 3 + 2] - mn) / (mx - mn);
		// make mesh aabb
	}


	glBindVertexArray(0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertex_count * sizeof(float) * 3, positions, GL_DYNAMIC_READ);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, normals);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertex_count * sizeof(float), norms, GL_DYNAMIC_READ);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void Model::draw() {
	if (vertex_count > 0) {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
		glBindVertexArray(0);
	}
}