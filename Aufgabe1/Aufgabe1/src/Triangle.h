#pragma once
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Triangle {
private:
	glm::vec4 corners[3];
	glm::vec4 center;
	glm::mat4 modelMatrix;
	float bvSize[3];
public:
	static constexpr float mesh[33] = {
		//positions				  //normals				  //uvs				//tangents
		-1.0f, -1.0f,  1.0f,	  0.0f,  0.0f, 1.0f,	  0.0f, 0.0f,		1.0f, 0.0f, 0.0f,
		 1.0f, -1.0f,  1.0f,	  0.0f,  0.0f, 1.0f,	  1.0f, 0.0f,		1.0f, 0.0f, 0.0f,
		 1.0f,  1.0f,  1.0f,	  0.0f,  0.0f, 1.0f,	  1.0f, 1.0f,		1.0f, 0.0f, 0.0f
	};
	static constexpr unsigned int indices[3] = {0, 1, 3};
	static constexpr glm::vec4 positions[3] = {
		glm::vec4(-1, -1,  1, 1),
		glm::vec4(1, -1,  1, 1),
		glm::vec4(1,  1,  1, 1)
	};
	Triangle() {};
	Triangle(const glm::mat4& mat);
	glm::mat4 getModelMat() { return modelMatrix; };
	glm::mat4 getModelMat(float zShift);
	float getCenterX() const { return center.x; };
	float getCenterY() const { return center.y; };
	float getCenterZ() const { return center.z; };
	glm::vec3 getCorner(int i) const{
		return glm::vec3(corners[i].x, corners[i].y, corners[i].z);
	}
};