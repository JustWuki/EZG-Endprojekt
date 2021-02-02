#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Triangle.h"
#include <iostream>

Triangle::Triangle(const glm::mat4& mat) {
	modelMatrix = mat;
	// calculate the global position of each corner of the triangle
	for (int i = 0; i < 3; i++) {
		corners[i] = mat * positions[i];
	}

	// calculate the limits of the bounding volume
	float maxX = -FLT_MAX, minX = FLT_MAX;
	float maxY = -FLT_MAX, minY = FLT_MAX;
	float maxZ = -FLT_MAX, minZ = FLT_MAX;
	for (int i = 0; i < 3; i++) {

		if (corners[i].x > maxX)
			maxX = corners[i].x;
		if (corners[i].x < minX)
			minX = corners[i].x;

		if (corners[i].y > maxY)
			maxY = corners[i].y;
		if (corners[i].y < minY)
			minY = corners[i].y;

		if (corners[i].z > maxZ)
			maxZ = corners[i].z;
		if (corners[i].z < minZ)
			minZ = corners[i].z;
	}
	bvSize[0] = (maxX - minX) / 2;
	bvSize[1] = (maxY - minY) / 2;
	bvSize[2] = (maxZ - minZ) / 2;

	// calculate the center of the bounding volume
	center = glm::vec4(minX + bvSize[0], minY + bvSize[1], minZ + bvSize[2], 1);

	//std::cout << corners[0].x << ", " << corners[1].x << ", " << corners[2].x << std::endl;
	//std::cout << corners[0].y << ", " << corners[1].y << ", " << corners[2].y << std::endl;
	//std::cout << corners[0].z << ", " << corners[1].z << ", " << corners[2].z << std::endl;
	//std::cout << std::endl;

	//std::cout << center.x << ", " << center.y << ", " << center.z << std::endl;
	//std::cout << bvSize[0] << ", " << bvSize[1] << ", " << bvSize[2] << std::endl;
}

glm::mat4 Triangle::getModelMat(float zShift) {
	return glm::translate(modelMatrix, glm::vec3(0, 0, zShift));
}