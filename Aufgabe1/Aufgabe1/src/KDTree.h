#pragma once
#include "Node.h"
#include "Triangle.h"
#include "Box.h"
#include <vector>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
class KDTree {
	
	void SortTriangles(std::vector<Triangle>& triangles, int from, int to, Node* currentNode);
	Triangle* visitNodes(Node* pNode, const float* point, const float* direction, float tmax);
	void fillBoxes(Node* pNode, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax);
public:
	Node* root = nullptr;
	glm::vec3 lastPoint = glm::vec3(0.0f);
	std::vector<Box> boxes;
	KDTree() : root() {};
	KDTree(std::vector<Triangle>& triangles, float minVal, float maxVal);
	KDTree(const KDTree& tree) : root(tree.root) {};
	Triangle* searchHit(const float* point, const float* direction, float tmax);
	bool testIntersection(const Triangle& triangle, glm::vec3 origin, glm::vec3 direction, glm::vec3& intersection);
	float orient(const glm::vec3& a, const  glm::vec3& b, const  glm::vec3& c, const  glm::vec3& d);
};