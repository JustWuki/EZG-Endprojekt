#pragma once
#include "Triangle.h"
class Node {
public:
	Node* leftChild;
	Node* rightChild;
	char splitPlane;	// either 'x', 'y' , 'z' or 'o' for not assigned yet
	float splitPos;
	Triangle* contents;
	Node() : leftChild(nullptr), rightChild(nullptr), splitPlane('o'), splitPos(0), contents(nullptr) {};
	Node(Node* leftChild,Node* rightChild,char splitPlane,float splitPos,Triangle* contents) : leftChild(nullptr), rightChild(nullptr), splitPlane('o'), splitPos(0), contents(nullptr) {};
	Node(const Node& node) {
		this->contents = node.contents;
		this->leftChild = node.leftChild;
		this->rightChild = node.rightChild;
		this->splitPlane = node.splitPlane;
		this->splitPos = node.splitPos;
	}
};