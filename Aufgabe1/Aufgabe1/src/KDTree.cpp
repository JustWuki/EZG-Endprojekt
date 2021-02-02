#include "KDTree.h"
#include "Triangle.h"
#include "Box.h"
#include <iostream>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

KDTree::KDTree(std::vector<Triangle>& triangles, float minVal, float maxVal) {
	root = new Node;
	SortTriangles(triangles, 0, triangles.size() - 1, root);
	fillBoxes(root, minVal - 1, maxVal + 1, minVal - 1, maxVal + 1, minVal - 1, maxVal + 1);
}

void KDTree::SortTriangles(std::vector<Triangle>& triangles, int from, int to, Node* currentNode) {
	// test if this would be a leaf node
	if (to == from) {
		// if to and from are the same value, this sublist is only one object long
		// the current node becomes a leaf node
		currentNode->leftChild = nullptr;
		currentNode->rightChild = nullptr;
		currentNode->splitPlane = 'o';
		currentNode->splitPos = 0.0f;
		currentNode->contents = &triangles[to];
		// std::cout << currentNode->contents->getCenterX() << " " << currentNode->contents->getCenterY() << " " << currentNode->contents->getCenterZ() << "\n";
		// we don't have to go on because this was a leaf node;
		return;
	}

	// find on which axis they are spread out more
	float maxX = -FLT_MAX, minX = FLT_MAX;
	float maxY = -FLT_MAX, minY = FLT_MAX;
	float maxZ = -FLT_MAX, minZ = FLT_MAX;
	for (int i = from; i <= to; i++) {

		if (triangles[i].getCenterX() > maxX)
			maxX = triangles[i].getCenterX();
		if (triangles[i].getCenterX() < minX)
			minX = triangles[i].getCenterX();

		if (triangles[i].getCenterY() > maxY)
			maxY = triangles[i].getCenterY();
		if (triangles[i].getCenterY() < minY)
			minY = triangles[i].getCenterY();

		if (triangles[i].getCenterZ() > maxZ)
			maxZ = triangles[i].getCenterZ();
		if (triangles[i].getCenterZ() < minZ)
			minZ = triangles[i].getCenterZ();
	}

	float deltaX = maxX - minX;
	float deltaY = maxY - minY;
	float deltaZ = maxZ - minZ;

	if (deltaX > deltaY && deltaX > deltaZ)
		currentNode->splitPlane = 'x';
	else if (deltaY > deltaX && deltaY > deltaZ)
		currentNode->splitPlane = 'y';
	else
		currentNode->splitPlane = 'z';

	// sort the array along the selected axis
	if (currentNode->splitPlane == 'x')
		std::sort(triangles.begin() + from, triangles.begin() + to + 1, [](const Triangle& first, const Triangle& second)->bool
			{
				return first.getCenterX() < second.getCenterX();
			});
	else if (currentNode->splitPlane == 'y')
		std::sort(triangles.begin() + from, triangles.begin() + to + 1, [](const Triangle& first, const Triangle& second)->bool
			{
				return first.getCenterY() < second.getCenterY();
			});
	else if (currentNode->splitPlane == 'z')
		std::sort(triangles.begin() + from, triangles.begin() + to + 1, [](const Triangle& first, const Triangle& second)->bool
			{
				return first.getCenterZ() < second.getCenterZ();
			});

	// find the median point on the more spreaded axis and the edges of the child lists
	int sum = from + to;
	int leftFrom = from;
	int rightTo = to;
	int leftTo = sum / 2;
	int rightFrom = leftTo + 1;
	if (sum % 2 == 1) {
		if (currentNode->splitPlane == 'x')
			currentNode->splitPos = (triangles[sum / 2].getCenterX() + triangles[sum / 2 + 1].getCenterX()) / 2;
		else if(currentNode->splitPlane == 'y')
			currentNode->splitPos = (triangles[sum / 2].getCenterY() + triangles[sum / 2 + 1].getCenterY()) / 2;
		else if (currentNode->splitPlane == 'z')
			currentNode->splitPos = (triangles[sum / 2].getCenterZ() + triangles[sum / 2 + 1].getCenterZ()) / 2;

		//std::cout << currentNode.splitPlane << " " << currentNode.splitPos << std::endl;
		//std::cout << leftFrom << " " << leftTo << " " << rightFrom << " " << rightTo << std::endl;
	}
	else {
		if (currentNode->splitPlane == 'x')
			currentNode->splitPos = (triangles[sum / 2].getCenterX());
		else if (currentNode->splitPlane == 'y')
			currentNode->splitPos = (triangles[sum / 2].getCenterY());
		else if (currentNode->splitPlane == 'z')
			currentNode->splitPos = (triangles[sum / 2].getCenterZ());

		currentNode->contents = &triangles[sum / 2];
		leftTo = sum / 2 - 1;
		rightFrom = sum / 2 + 1;

		//std::cout << currentNode.splitPlane << " " << currentNode.splitPos << " "<< sum/2 << std::endl;
		//std::cout << leftFrom << " " << leftTo << " " << rightFrom << " " << rightTo << std::endl;
	}

	//std::cout << "----------------------" << std::endl;

	// call create new childNodes and call SortTriangles recursively using them and the new boundaries in the list
	Node* leftChild = new Node();
	Node* rightChild = new Node();
	currentNode->leftChild = leftChild;
	currentNode->rightChild = rightChild;

	// we need to create new vectors because SortTriangles will change the order
	std::vector<Triangle>*  leftList = new std::vector<Triangle>();
	std::vector<Triangle>* rightList = new std::vector<Triangle>();
	for (int i = leftFrom; i <= leftTo; i++) {
		leftList->push_back(triangles[i]);
	}

	for (int i = rightFrom; i <= rightTo; i++) {
		rightList->push_back(triangles[i]);
	}

	// start with the left side
	SortTriangles(*leftList, 0, leftList->size() - 1, leftChild);

	// then make the right side
	SortTriangles(*rightList, 0, rightList->size() - 1, rightChild);
}

Triangle* KDTree::searchHit(const float* point, const float* direction, float tmax){
	return visitNodes(root, point, direction, tmax);
}

Triangle* KDTree::visitNodes(Node* pNode, const float* point, const float* direction, float tmax) {

	// we have to check if there are contents saved in this node
	if (pNode->contents != nullptr) {
		// if there are contents saved in this node call the funciton to test if it hit with the actual geometry
		// TODO add this function
		glm::vec3 originFunk = glm::vec3(point[0],point[1], point[2]);
		glm::vec3 dirFunk = glm::vec3(direction[0], direction[1], direction[2]);
		glm::vec3 output;
		if (testIntersection(*(pNode->contents), originFunk, dirFunk, output)) {
			//std::cout << "hit triangle " << output.x << " " << output.y << " " << output.z << std::endl;
			lastPoint = output;
			return pNode->contents;
		}
	}

	// if it does not have children, return
	if (pNode->leftChild == nullptr && pNode->rightChild == nullptr)
		return nullptr;

	// we need to find which child we need to go inside first
	char dimension = pNode->splitPlane - 120; // minus 120 to get the 0, 1 or 2 from the ASCII value from x, y and z
	bool smallerFirst = point[dimension] < pNode->splitPos;

	// test if the ray was parallel to this plane (very unprobable)
	if (direction[dimension] == 0.0f) {
		// return if a hit will be found in only the first child
		if (smallerFirst)
			return visitNodes(pNode->leftChild, point, direction, tmax);
		else
			return visitNodes(pNode->rightChild, point, direction, tmax);
	}
	else {
		// calculate the intersection t
		float t = (pNode->splitPos - point[dimension]) / direction[dimension];

		// if the calculated t is in the ray segment
		if (0.0f <= t && t < tmax) {
			// test first if something is found in the first child
			Triangle* result;
			if (smallerFirst)
				result = visitNodes(pNode->leftChild, point, direction, t);
			else
				result = visitNodes(pNode->rightChild, point, direction, t);

			// if something was found, return it without searching the second child
			if (result != nullptr) {
				return result;
			}
			else {
				// visit the second child taking into account the new line segment (only the second part of the ray)
				float newPoint[3] = { point[0] + t * direction[0], point[1] + t * direction[1] , point[2] + t * direction[2] };
				if (smallerFirst)
					return visitNodes(pNode->rightChild, newPoint, direction, tmax - t);
				else
					return visitNodes(pNode->leftChild, newPoint, direction, tmax - t);
			}
		}
		else {
			// return if a hit will be found in only the first child
			if (smallerFirst)
				return visitNodes(pNode->leftChild, point, direction, tmax);
			else
				return visitNodes(pNode->rightChild, point, direction, tmax);
		}
	}
}

// method that tests if there exists an intersection between the ray and a triangle
// returns true if hit, and the coordinates are stored int he intersecion reference
bool KDTree::testIntersection(const Triangle& triangle, glm::vec3 origin, glm::vec3 direction, glm::vec3& intersection) {
	// we calculate som working variables
	const float eps = 0.0001f;
	glm::vec3 edge1 = triangle.getCorner(1) - triangle.getCorner(0);
	glm::vec3 edge2 = triangle.getCorner(2) - triangle.getCorner(0);
	glm::vec3 normal = glm::cross(edge1, edge2);
	glm::normalize(normal);
	float d = glm::dot(-normal, triangle.getCorner(0));
	float check = glm::dot(normal, direction);

	// it we are parallel to the triangle return false
	if (std::abs(check) < eps) {
		return false;
	}
	// calculate the t that intersectc with the triangle plane
	float t = -(glm::dot(normal, origin) + d) / check;
	// save the point
	intersection = origin + t * direction;

	// we will check if the point is inside the triangle
	// we will add 1 to each side when the point is on that side
	// if one side gets 3 or more points the point is inside the three planes
	int sideA = 0;
	int sideB = 0;

	for (int i = 0; i < 3; i++) {
		const glm::vec3& a = triangle.getCorner(i);
		const glm::vec3& b = triangle.getCorner((i + 1) % 3);
		const glm::vec3 c = triangle.getCorner(i) + normal;
		float o = orient(a, b, c, intersection);
		if (o < -eps) {
			sideA++;
		}
		else if (o > eps) {
			sideB++;
		}
		else {
			sideA++;
			sideB++;
		}
	}

	if (sideA >= 3 || sideB >= 3) {
		return true;
	}
	return false;
}

// method to calculate if a point is above or below a plane
float KDTree::orient(const glm::vec3& a, const  glm::vec3& b, const  glm::vec3& c, const  glm::vec3& d) {
	return glm::dot((a - d), glm::cross((b - d), (c - d)));
}

void KDTree::fillBoxes(Node* pNode, float xMin, float xMax, float yMin, float yMax, float zMin, float zMax) {
	// make and save this box
	Box* thisBox = new Box(xMin, xMax, yMin, yMax, zMin, zMax);
	boxes.push_back(*thisBox);

	// has this node children?
	if (pNode->splitPlane == 'o') {
		// it has no children, return
		return;
	}

	// find this node's split plane
	// and call the method recursively on both children
	if (pNode->splitPlane == 'x') {
		fillBoxes(pNode->leftChild, xMin, pNode->splitPos, yMin, yMax, zMin, zMax);
		fillBoxes(pNode->rightChild, pNode->splitPos, xMax, yMin, yMax, zMin, zMax);
	}
	else if (pNode->splitPlane == 'y') {
		fillBoxes(pNode->leftChild, xMin, xMax, pNode->splitPos, yMax, zMin, zMax);
		fillBoxes(pNode->rightChild, xMin, xMax, yMin, pNode->splitPos, zMin, zMax);
	}
	else {
		fillBoxes(pNode->leftChild, xMin, xMax, yMin, yMax, pNode->splitPos, zMax);
		fillBoxes(pNode->rightChild, xMin, xMax, yMin, yMax, zMin, pNode->splitPos);
	}
}
