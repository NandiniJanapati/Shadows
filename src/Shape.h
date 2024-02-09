#pragma once
#include <vector>
#include <glm/glm.hpp>
#include "Program.h"

class Shape
{
private:
	std::vector<float> posBuff;
	std::vector<float> norBuff;
	std::vector<float> texBuff;


	GLuint posBuffID;
	GLuint norBuffID;
	GLuint texBuffID;

	glm::mat4 modelMatrix;

public:
	Shape();
	~Shape();

	void LoadModel(char* name);
	void Draw(Program& prog);
	void Init();


	std::vector<float>& GetPosBuff() { return posBuff; };
	std::vector<float>& GetNorBuff() { return norBuff; };
	glm::mat4& GetModelMatrix() { return modelMatrix; };

	void SetModel(glm::mat4 matrix);

	//TODO: add material information
	glm::vec3 ka;
	glm::vec3 kd;
	glm::vec3 ks;
	float s;

};

