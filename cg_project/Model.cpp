#include "Model.hpp"
struct Vertex {
	float x, y, z;
};
const int MXN = 1e6 + 5;
Vertex positions[MXN];
Vertex norms[MXN];
void Model::initializeBuffers() {
	glGenBuffers(1, &EBO);
	glGenBuffers(1, &VBO);
	glGenVertexArrays(1, &VAO);

	glGenBuffers(1, &triangles);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, triangles);

	glGenBuffers(1, &normals);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, normals);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, normals);

}
Model::Model() {
	initializeBuffers();
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


		positions[i].x = x;
		positions[i].y = y;
		positions[i].z = z;

		norms[i].x = nx;
		norms[i].y = ny;
		norms[i].z = nz;

		// make mesh aabb
	}

	for (int i = 0; i < vertex_count; ++i) {
		
		// x -> x - mn -> (x - mn) / (mx - mn)
		positions[i].x = (positions[i].x - mn) / (mx - mn); //[mn,mx] -> [0,mx-mn] -> 
		positions[i].y = (positions[i].y - mn) / (mx - mn); 
		positions[i].z = (positions[i].z - mn) / (mx - mn);

		positions[i].x = 0.5; //[mn,mx] -> [0,mx-mn] -> 
		positions[i].y = 0.5;
		positions[i].z = 0.5;
		// make mesh aabb
	}
	glBindVertexArray(0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, triangles);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Vertex) * vertex_count, positions, GL_DYNAMIC_READ);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, normals);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(Vertex) * vertex_count, norms, GL_DYNAMIC_READ);
}
void Model::draw() {
	if (vertex_count > 0) {
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, vertex_count);
		glBindVertexArray(0);
	}
}