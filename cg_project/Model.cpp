#include "Model.hpp"
const int MXN = 1e6 + 5;
float positions[MXN];
float norms[MXN];
float mx = -1e38;
float mn = 1e38;
mat4 Model::meshToField() const {
	return mat4::scale(vec3(1.0f / (mx - mn))) * mat4::trans(vec3(-mn));
}
mat4 Model::fieldToMesh() const {
	return meshToField().inverse();
}
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
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, triangles);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, normals);
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
	for(int i = 0; i < vertex_count; ++i) {
		float x = vertices[i * 8 + 0];
		float y = vertices[i * 8 + 1];
		float z = vertices[i * 8 + 2];
		
		mx = std::fmax(2 * x, mx);
		mx = std::fmax(2 * y, mx);
		mx = std::fmax(2 * z, mx);
		mn = std::fmin(2 * x, mn);
		mn = std::fmin(2 * y, mn);
		mn = std::fmin(2 * z, mn);

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
	for (int i = 0; i < vertex_count; ++i) {
		
		// x -> x - mn -> (x - mn) / (mx - mn)
		positions[i * 3 + 0] = (positions[i * 3 + 0] - mn) / (mx - mn); //[mn,mx] -> [0,mx-mn] -> 
		positions[i * 3 + 1] = (positions[i * 3 + 1] - mn) / (mx - mn); 
		positions[i * 3 + 2] = (positions[i * 3 + 2] - mn) / (mx - mn);
		// make mesh aabb
	}
	for (int i = 0; i < vertex_count; i += 3) {
		float nx = vertices[i * 8 + 3];
		float ny = vertices[i * 8 + 4];
		float nz = vertices[i * 8 + 5];

		norms[i + 0] = nx;
		norms[i + 1] = ny;
		norms[i + 2] = nz;
	}



	glBindVertexArray(0);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertex_count * sizeof(float) * 3, positions, GL_DYNAMIC_READ);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, normals);
	glBufferData(GL_SHADER_STORAGE_BUFFER, vertex_count * sizeof(float), norms, GL_DYNAMIC_READ);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	maximum_vertex_distance = mx;
	minimum_vertex_distance = mn;

	box_size = std::fmax(mx - mn, 1e-6f);
}

void Model::draw() {
	if (vertex_count > 0) {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
		glBindVertexArray(0);
	}
}