#include "Shape.h"
#include <GL/glew.h>
#include <string>
#include <iostream>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"



Shape::Shape()
{
	posBuffID = 0;
	norBuffID = 0;
	texBuffID = 0;

	modelMatrix = glm::mat4(1.0f);
}

Shape::~Shape()
{

}

void Shape::SetModel(glm::mat4 matrix) 
{
	modelMatrix = matrix;
}

void Shape::LoadModel(char* name)
{
	// Taken from Shinjiro Sueda with slight modification
	std::string meshName(name);
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string errStr;
	bool rc = tinyobj::LoadObj(&attrib, &shapes, &materials, &errStr, meshName.c_str());
	if (!rc) {
		std::cerr << errStr << std::endl;
	}
	else {
		// Some OBJ files have different indices for vertex positions, normals,
		// and texture coordinates. For example, a cube corner vertex may have
		// three different normals. Here, we are going to duplicate all such
		// vertices.
		// Loop over shapes
		for (size_t s = 0; s < shapes.size(); s++) {
			// Loop over faces (polygons)
			size_t index_offset = 0;
			for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
				size_t fv = shapes[s].mesh.num_face_vertices[f];
				// Loop over vertices in the face.
				for (size_t v = 0; v < fv; v++) {
					// access to vertex
					tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
					posBuff.push_back(attrib.vertices[3 * idx.vertex_index + 0]);
					posBuff.push_back(attrib.vertices[3 * idx.vertex_index + 1]);
					posBuff.push_back(attrib.vertices[3 * idx.vertex_index + 2]);
					if (!attrib.normals.empty()) {
						norBuff.push_back(attrib.normals[3 * idx.normal_index + 0]);
						norBuff.push_back(attrib.normals[3 * idx.normal_index + 1]);
						norBuff.push_back(attrib.normals[3 * idx.normal_index + 2]);
					}
					if (!attrib.texcoords.empty()) {
						texBuff.push_back(attrib.texcoords[2 * idx.texcoord_index + 0]);
						texBuff.push_back(attrib.texcoords[2 * idx.texcoord_index + 1]);
					}
				}
				index_offset += fv;
				// per-face material (IGNORE)
				shapes[s].mesh.material_ids[f];
			}
		}
	}
}

void Shape::Init()
{
	glGenBuffers(1, &posBuffID);
	glBindBuffer(GL_ARRAY_BUFFER, posBuffID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * posBuff.size(), &posBuff[0], GL_STATIC_DRAW);

	if (!norBuff.empty())
	{
		glGenBuffers(1, &norBuffID);
		glBindBuffer(GL_ARRAY_BUFFER, norBuffID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * norBuff.size(), &norBuff[0], GL_STATIC_DRAW);
	}

	if (!texBuff.empty())
	{
		glGenBuffers(1, &texBuffID);
		glBindBuffer(GL_ARRAY_BUFFER, texBuffID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * texBuff.size(), &texBuff[0], GL_STATIC_DRAW);
	}

}

void Shape::Draw(Program& prog)
{
	glBindBuffer(GL_ARRAY_BUFFER, posBuffID);
	GLint aLoc = glGetAttribLocation(prog.GetID(), "vPositionModel");
	glEnableVertexAttribArray(aLoc);
	glVertexAttribPointer(aLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);

	if (!norBuff.empty())
	{
		glBindBuffer(GL_ARRAY_BUFFER, norBuffID);
		GLint aLoc = glGetAttribLocation(prog.GetID(), "vNormalModel");
		glEnableVertexAttribArray(aLoc);
		glVertexAttribPointer(aLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
	}

	if (!texBuff.empty())
	{
		glBindBuffer(GL_ARRAY_BUFFER, texBuffID);
		GLint aLoc = glGetAttribLocation(prog.GetID(), "vTextureModel");
		glEnableVertexAttribArray(aLoc);
		glVertexAttribPointer(aLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
	}

	glDrawArrays(GL_TRIANGLES, 0, posBuff.size() / 3);
}