#pragma once
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
class Box {
public:
    static constexpr float points[48] = {
        0,0,0,
        0,0,-1,
        -1,0,-1,
        -1,0,0,
        0,0,0,
        0,-1,0,
        0,-1,-1,
        0,0,-1,
        -1,0,-1,
        -1,-1,-1,
        0,-1,-1,
        0,-1,0,
        -1,-1,0,
        -1,0,0,
        -1,-1,0,
        -1,-1,-1
    };
	float xMin, xMax, yMin, yMax, zMin, zMax;
	Box() : xMin(0), xMax(0), yMin(0), yMax(0), zMin(0), zMax(0) {};
	Box(float xMin, float xMax, float yMin, float yMax, float zMin, float zMax) {
		this->xMin = xMin;
		this->xMax = xMax;
		this->yMin = yMin;
		this->yMax = yMax;
		this->zMin = zMin;
		this->zMax = zMax;
	}

	std::vector<glm::vec3> getCorners() {
		std::vector<glm::vec3>* toReturn = new std::vector<glm::vec3>(8);
		toReturn->at(0) = *(new glm::vec3(xMin, yMin, zMin));
		toReturn->at(1) = *(new glm::vec3(xMin, yMax, zMin));
		toReturn->at(2) = *(new glm::vec3(xMin, yMin, zMax));
		toReturn->at(3) = *(new glm::vec3(xMin, yMax, zMax));
		toReturn->at(4) = *(new glm::vec3(xMax, yMin, zMin));
		toReturn->at(5) = *(new glm::vec3(xMax, yMax, zMin));
		toReturn->at(6) = *(new glm::vec3(xMax, yMin, zMax));
		toReturn->at(7) = *(new glm::vec3(xMax, yMax, zMax));
		return *toReturn;
	}

    glm::mat4 getTransformMatrix(glm::mat4& model) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xMax, yMax, zMax));
        //model = glm::translate(model, glm::vec3(11, 11, 11));
        model = glm::scale(model, glm::vec3(xMax - xMin, yMax - yMin, zMax - zMin));
        return model;
    }

};