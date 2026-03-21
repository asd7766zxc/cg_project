#pragma once
#include <tiny_obj_loader.h>
#include <iostream>

class ModelLoader {
public:
    float *vertices;
    int vertex_size = 0;
    ModelLoader(std::string inputfile){
		tinyobj::attrib_t attrib;
		std::vector<tinyobj::shape_t> shapes;
		std::vector<tinyobj::material_t> materials;
		std::string warn;
		bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, inputfile.c_str());
		if (!warn.empty()) {
			std::cout << warn << std::endl;
		}
        int vertindex = 0;
        // Loop over shapes
        for (size_t s = 0; s < shapes.size(); s++) {
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
                vertex_size += fv;
            }
        }
        vertices = new float[vertex_size * 8];
        for (size_t s = 0; s < shapes.size(); s++) {
            // Loop over faces(polygon)
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
                size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

                // Loop over vertices in the face.
                for (size_t v = 0; v < fv; v++) {
                    // access to vertex
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                    float vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                    float vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                    float vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
                    float nx = 0;
                    float ny = 0;
                    float nz = 0;
                    float tx = 0;
                    float ty = 0;
                    if (idx.normal_index >= 0) {
                          nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                          ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                          nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    }

                    if (idx.texcoord_index >= 0) {
                         tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                         ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    }

                    vertices[vertindex * 8 + 0] = vx;
                    vertices[vertindex * 8 + 1] = vy;
                    vertices[vertindex * 8 + 2] = vz;

                    vertices[vertindex * 8 + 3] = nx;
                    vertices[vertindex * 8 + 4] = ny;
                    vertices[vertindex * 8 + 5] = nz;

                    vertices[vertindex * 8 + 6] = tx;
                    vertices[vertindex * 8 + 7] = ty;

                    ++vertindex;
                }
                index_offset += fv;

                // per-face material
                //shapes[s].mesh.material_ids[f];
            }
        }
	}
};