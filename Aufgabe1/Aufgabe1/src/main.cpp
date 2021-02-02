#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Shader.h"

#include <iostream>
#include <vector>
#include <algorithm> 

#include <random>
#include "Triangle.h"
#include "KDTree.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
unsigned int loadTexture(const char* path);
void restartScene();
void renderCube();
void renderScene(const Shader& shader, const glm::vec3 cubePos[]);

// calculation functions
int calcCorrectIndex(int index);
std::vector<glm::vec3> calcTangents(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
glm::vec3 calcPoint(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 tang1, glm::vec3 tang2);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;

std::vector<glm::vec3> tangents;
//num of points for camerapath
const int CAMERPATHLENGTH = 20;
//increment for T
float increment = 0.005f;
float bumpiness = 1.0f;
int samples = 4;
bool showGrid = false;

unsigned int planeVAO;

unsigned int triangleVAO;
unsigned int triangleVBO;
unsigned int pointVAO;
unsigned int pointVBO;
float point[3] = { 0.0f, 0.0f,  0.0f };
unsigned int wireCubeVAO;
unsigned int wireCubeVBO;

//kd tree vars
int triangleAmount = 40;
int maxVal = 10;
int minVal = -maxVal;
std::vector<Triangle> triangles;
KDTree tree;
Triangle* lastResult;

//mouse values
double mouseX, mouseY;
double clickX = -10, clickY = -10;

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
	{
		clickX = mouseX;
		clickY = mouseY;
	}
}

glm::vec3 CreateRay(const glm::mat4& projection, const glm::mat4& view) {
	float x = (2.0f * clickX) / SCR_WIDTH - 1.0f;
	float y = 1.0f - (2.0f * clickY) / SCR_HEIGHT;
	float z = 1.0f;
	glm::vec3 ray_nds = glm::vec3(x, y, z);

	glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

	glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;

	ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

	glm::vec4 result = (glm::inverse(view) * ray_eye);
	glm::vec3 ray_wor = glm::vec3(result.x, result.y, result.z);
	// don't forget to normalise the vector at some point
	ray_wor = glm::normalize(ray_wor);
	return ray_wor;
}

void printUsage() {
	std::cerr << "Usage: Aufgabe1.exe --samples [sampling mode] --triangles triangleAmount --extremes " << std::endl;
}

int main(int argc, char* argv[])
{
	for (int i = 1; i < argc; i++) {
		if (std::string(argv[i]) == "--samples") {
			if (i + 1 < argc) {
				if (std::stoi(argv[i + 1]) >= 0) {
					samples = std::stoi(argv[i + 1]);
				}
				else {
					printUsage();
					return 1;
				}
			}
			else {
				printUsage();
				return 1;
			}
		}
		else if (std::string(argv[i]) == "--triangles") {
			if (i + 1 < argc) {
				if (std::stoi(argv[i + 1]) >= 0) {
					triangleAmount = std::stoi(argv[i + 1]);
				}
				else {
					printUsage();
					return 1;
				}
			}
			else {
				printUsage();
				return 1;
			}
		}
		else if (std::string(argv[i]) == "--extremes") {
			if (i + 1 < argc) {
				if (std::stoi(argv[i + 1]) >= 0) {
					maxVal = std::stoi(argv[i + 1]);
					minVal = -maxVal;
				}
				else {
					printUsage();
					return 1;
				}
			}
			else {
				printUsage();
				return 1;
			}
		}
	}

	//create random triangles in range [minValue, maxValue]
	std::default_random_engine e1(1234);
	std::uniform_int_distribution<int> uniform_dist(minVal, maxVal);

	for (int i = 0; i < triangleAmount; i++) {
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, glm::vec3(uniform_dist(e1), uniform_dist(e1), uniform_dist(e1)));
		model = glm::rotate(model, (float)uniform_dist(e1), glm::vec3(1, 0, 0));
		//model = glm::rotate(model, (float)90, glm::vec3(1, 0, 0));
		model = glm::rotate(model, (float)uniform_dist(e1), glm::vec3(0, 1, 0));
		model = glm::rotate(model, (float)uniform_dist(e1), glm::vec3(0, 0, 1));
		triangles.push_back(Triangle(model));
	}

	tree = KDTree(triangles, minVal, maxVal);

    // glfw: initialize and configure
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, samples);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Window", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetKeyCallback(window, keyboardCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

	// build and compile our shader program
	Shader ourShader("src/shader.vs", "src/shader.fs");
	//Shader ourShader("src/VertexShader.vert", "src/FragmentShader.frag");
	Shader depthShader("src/depthShader.vs", "src/depthShader.fs");
	Shader pointShader("src/SimpleVertexShader.vs", "src/SimpleColorFragmentShader.fs");

    // configure global opengl state
    glEnable(GL_DEPTH_TEST);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_PROGRAM_POINT_SIZE);

    // world space positions cubes
    glm::vec3 cubePositions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(0.0f,  2.0f, 3.0f),
		glm::vec3(0.0f,  0.0f, 6.0f),
		glm::vec3(0.0f,  1.0f, 8.0f),
		glm::vec3(0.0f,  0.0f, 10.0f),
		glm::vec3(0.0f,  -3.0f, 12.0f),
		glm::vec3(5.0f,  0.0f, 3.0f),
		glm::vec3(5.0f,  0.0f, 6.0f),
		glm::vec3(5.0f,  5.0f, 8.0f),
		glm::vec3(5.0f,  0.0f, 10.0f),
		glm::vec3(7.0f,  0.0f, 12.0f),
		glm::vec3(-5.0f,  0.0f, 3.0f),
		glm::vec3(-8.0f,  0.0f, 6.0f),
		glm::vec3(-6.0f,  3.0f, 8.0f),
		glm::vec3(-5.0f,  0.0f, 10.0f),
		glm::vec3(-6.0f,  0.0f, 12.0f),
		glm::vec3(-1, 2, -1.0f)
    };

    // world space position of path
	glm::vec3 pathPos[] = {
		glm::vec3(0.0f,  5.0f,  -3.0f),
		glm::vec3(1.0f,  3.0f,  -1.0f),
		glm::vec3(2.0f,  3.0f,  1.0f),
		glm::vec3(2.0f,  3.0f,  0.0f),
		glm::vec3(4.0f,  3.0f,  4.0f),
		glm::vec3(3.0f,  2.0f,  8.0f),
		glm::vec3(2.0f,  1.0f,  10.0f),
		glm::vec3(1.0f,  1.0f,  12.0f),
		glm::vec3(4.0f,  0.0f,  14.0f),
		glm::vec3(2.0f,  2.0f,  20.0f),
		glm::vec3(0.0f,  3.0f,  14.0f),
		glm::vec3(-2.0f,  5.0f,  12.0f),
		glm::vec3(-2.0f,  4.0f,  10.0f),
		glm::vec3(-2.0f,  3.0f,  8.0f),
		glm::vec3(-2.0f,  2.0f,  6.0f),
		glm::vec3(-2.0f,  0.0f,  4.0f),
		glm::vec3(-2.0f,  0.0f,  2.0f),
		glm::vec3(-2.0f,  0.0f,  0.0f),
		glm::vec3(-2.0f,  0.0f,  -2.0f),
		glm::vec3(-1.0f,  0.0f,  -2.0f)
	};

	glm::vec3 lookDir[]{
        glm::vec3(0.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 1.0f),
        glm::vec3(0.3f, 0.0f, 1.0f),
        glm::vec3(0.1f, 0.0f, 1.0f),
        glm::vec3(0.1, 0, 0.7),
        glm::vec3(0.3, 0, 0.5),
        glm::vec3(-1, 0, -1),
        glm::vec3(-0.4, 0, -0),
        glm::vec3(-0.7, 0, -1),
        glm::vec3(-1, 0, -1),
        glm::vec3(-1, 0, -0.8),
        glm::vec3(-1, 0, -0.5),
        glm::vec3(-1, 0, -0.3),
        glm::vec3(-1, 0, 0),
        glm::vec3(-1, 0, 0.1),
        glm::vec3(-1, 0, 0.5),
        glm::vec3(-1, 0, 1),
        glm::vec3(-1, 0, 1),
        glm::vec3(0, 0, 1),
	};

	float planeVertices[] = {
		//// positions            // normals
		// 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,
		//-10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,
		//-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,

		// 10.0f, -0.5f,  10.0f,  0.0f, 1.0f, 0.0f,
		//-10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f,
		// 10.0f, -0.5f, -10.0f,  0.0f, 1.0f, 0.0f

		 // positions            // normals       
		 /*25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f, 
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f*/

		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f, 0.0f,
		-25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f, 1.0f, 0.0f, 0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f, 1.0f, 0.0f, 0.0f,
																 
		 25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f, 1.0f, 0.0f, 0.0f,
		-25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f, 1.0f, 0.0f, 0.0f,
		 25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f, 1.0f, 0.0f, 0.0f
	};

	glGenVertexArrays(1, &triangleVAO);
	glGenBuffers(1, &triangleVBO);
	glBindVertexArray(triangleVAO);
	glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Triangle::mesh), Triangle::mesh, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glBindVertexArray(0);

	// plane VAO
	unsigned int planeVBO;
	glGenVertexArrays(1, &planeVAO);
	glGenBuffers(1, &planeVBO);
	glBindVertexArray(planeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertices), planeVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glBindVertexArray(0);

	// we configurate the point VAO of the point
	glGenVertexArrays(1, &pointVAO);
	glGenBuffers(1, &pointVBO);
	glBindVertexArray(pointVAO);
	glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(point), point, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// we configurate the cube VAO for the boxes
	glGenVertexArrays(1, &wireCubeVAO);
	glGenBuffers(1, &wireCubeVBO);
	glBindVertexArray(wireCubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, wireCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Box::points), Box::points, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// load textures
	unsigned int diffuseMap = loadTexture("src/brickwall.jpg");
	unsigned int normalMap = loadTexture("src/brickwall_normal.jpg");

	// configure depth map FBO
	unsigned int depthMapFBO;
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	unsigned int depthMap;
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	ourShader.use();
	ourShader.setInt("diffuseTexture", 0);
	ourShader.setInt("normalMap", 1);
	ourShader.setInt("shadowMap", 2);
	
	// lighting info
	//glm::vec3 lightPos(20.0f, 100.0f, 120.0f);
	glm::vec3 lightPos(0, 50, 0);

	// vars for camerapath calculation
	float t = 0.0f;
	int currentPointIndex = 1;
	glm::quat lookDirQuaternions[20];
	glm::vec3 initialOrientation = glm::vec3(0.0f, 0.0f, -1.0f);

    //transform lookDir vectors to Quaternions
	for (int i = 0; i < 20; i++) {
		lookDirQuaternions[i] = glm::rotation(glm::normalize(initialOrientation), glm::normalize(lookDir[i]));
	}

    // render loop
    while (!glfwWindowShouldClose(window))
    {
		if (t < 1) {
            t += increment;
		}
		else {
			currentPointIndex = calcCorrectIndex(currentPointIndex);
			t = 0.0f;
		}

		if (currentPointIndex > CAMERPATHLENGTH) {
			currentPointIndex = 1;
		}  

        // input
        processInput(window);
		glfwGetCursorPos(window, &mouseX, &mouseY);

		// render
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// 1. render depth of scene to texture (from light's perspective)
		ourShader.setFloat("bumpiness", bumpiness);
		glm::mat4 lightProjection, lightView;
		glm::mat4 lightSpaceMatrix;
		float near_plane = 0.1f, far_plane = 100.0f;
		lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
		lightView = glm::lookAt(lightPos, glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0, 1.0, 0.0));
		lightSpaceMatrix = lightProjection * lightView;
		// render scene from light's point of view
		depthShader.use();
		depthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);

		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		glCullFace(GL_FRONT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		renderScene(depthShader, cubePositions);
		glCullFace(GL_BACK);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // render scene second time normally
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		ourShader.use();
		// pass projection matrix to shader
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		ourShader.setMat4("projection", projection);

		tangents = calcTangents(pathPos[currentPointIndex - 1], pathPos[currentPointIndex], pathPos[currentPointIndex + 1], pathPos[currentPointIndex + 2]);
		//calculate the helper quats for p0 and p1
		glm::quat helpQuat1 = glm::intermediate(lookDirQuaternions[currentPointIndex - 1], lookDirQuaternions[currentPointIndex], lookDirQuaternions[currentPointIndex + 1]);
		glm::quat helpQuat2 = glm::intermediate(lookDirQuaternions[currentPointIndex], lookDirQuaternions[currentPointIndex + 1], lookDirQuaternions[currentPointIndex + 2]);
		// calculate the point the camera will move to and the direction it will look
		glm::vec3 movePoint = calcPoint(t, pathPos[currentPointIndex], pathPos[currentPointIndex + 1], tangents[0], tangents[1]);
		glm::quat lookQuat = glm::squad(lookDirQuaternions[currentPointIndex], lookDirQuaternions[currentPointIndex + 1], helpQuat1, helpQuat2, t);

		// camera/view transformation
		//glm::mat4 view = glm::lookAt(movePoint, movePoint + lookQuat * initialOrientation, glm::vec3(0.0f, 1.0f, 0.0f));
		// wide view test
		//glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 2.0f), glm::vec3(0, 1, 0), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 3.0f, 40.0f), glm::vec3(0, 0, 3), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::vec3 cameraPos = glm::vec3(0.0f, 3.0f, 40.0f);
		// close up test
		glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 2.0f, 4.0f), glm::vec3(0, -5, 3), glm::vec3(0.0f, 1.0f, 0.0f));
		//glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 15.0f, 6.0f), glm::vec3(0, -5, 2), glm::vec3(0.0f, 1.0f, 0.0f));
		
		ourShader.setMat4("view", view);

		//ourShader.setVec3("objectColor", 0.2f, 0.5f, 0.31f);
		//ourShader.setVec3("viewPos", glm::vec3(0, 0, 0));
		ourShader.setVec3("viewPos", movePoint);
		ourShader.setVec3("lightPos", lightPos);
		ourShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, normalMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		renderScene(ourShader, cubePositions);

		// we test if we need to create a new raycast and if so search the tree for a hit
		if (clickX > 0 && clickY > 0 && clickX < SCR_WIDTH && clickY < SCR_HEIGHT) {
			//std::cout << clickX << " " << clickY << std::endl;
			glm::vec3 ray = CreateRay(projection, view);
			//std::cout << ray.x << " " << ray.y << " " << ray.z << std::endl;

			float camPos[3] = { movePoint.x, movePoint.y, movePoint.z };
			//float camPos[3] = { cameraPos.x, cameraPos.y, cameraPos.z };
			float rayDir[3] = { ray.x, ray.y, ray.z };

			Triangle* result = tree.searchHit(camPos, rayDir, 100);
			if (result != nullptr) {
				lastResult = result;
			}

			clickX = -10;
			clickY = -10;
			std::cout << tree.lastPoint.x << " " << tree.lastPoint.y << " " << tree.lastPoint.z << std::endl;
		}

		if (tree.lastPoint.x != 0 && tree.lastPoint.y != 0 && tree.lastPoint.z != 0) {
			glBindVertexArray(triangleVAO);
			pointShader.use();
			pointShader.setMat4("projection", projection);
			pointShader.setMat4("view", view);
			pointShader.setVec3("color", glm::vec3(0.0f, 1.0f, 0.0f));
			pointShader.setMat4("model", lastResult->getModelMat(0.001));
			glDrawArrays(GL_TRIANGLES, 0, 3);
			pointShader.setMat4("model", lastResult->getModelMat(-0.001));
			glDrawArrays(GL_TRIANGLES, 0, 3);

			glBindVertexArray(pointVAO);
			pointShader.use();
			pointShader.setMat4("projection", projection);
			pointShader.setMat4("view", view);
			glm::mat4 model = glm::mat4(1.0f);
			// we draw the intersection point
			model = glm::translate(model, tree.lastPoint);
			model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.01f));
			pointShader.setVec3("color", glm::vec3(1.0f, 0.0f, 0.0f));
			pointShader.setMat4("model", model);
			glDrawArrays(GL_POINTS, 0, 1);
		}

		if (showGrid) {
			glBindVertexArray(pointVAO);
			pointShader.use();
			pointShader.setMat4("projection", projection);
			pointShader.setMat4("view", view);
			glm::mat4 model = glm::mat4(1.0f);
			glBindVertexArray(wireCubeVAO);
			for (auto box : tree.boxes) {
				model = glm::mat4(1.0f);
				box.getTransformMatrix(model);
				pointShader.setVec3("color", glm::vec3(0.0f, 0.0f, 1.0f));
				pointShader.setMat4("model", model);
				glDrawArrays(GL_LINE_STRIP, 0, 16);
			}
		}
		ourShader.use();

        // glfw: swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // de-allocate all resources
	glDeleteVertexArrays(1, &planeVAO);
	glDeleteBuffers(1, &planeVBO);

    // glfw: terminate
    glfwTerminate();
    return 0;
}

void renderScene(const Shader& shader, const glm::vec3 cubePos[])
{
	//floor plane
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -2.0f, 0.0f));
	shader.setMat4("model", model);
	glBindVertexArray(planeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	//triangles
	glBindVertexArray(triangleVAO);
	for (auto triangle : triangles) {
		shader.setMat4("model", triangle.getModelMat());
		glDrawArrays(GL_TRIANGLES, 0, 3);
	}

	//cubes
	//for (unsigned int i = 0; i < 17; i++)
	//{
	//	// calculate the model matrix for each object and pass it to shader before drawing
	//	glm::mat4 model = glm::mat4(1.0f);
	//	model = glm::translate(model, cubePos[i]);
	//	shader.setMat4("model", model);
	//	renderCube();
	//}
}

unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	if (cubeVAO == 0)
	{
		// vertex data
		float vertices[] = {
			// positions          // normals          // texture	// tangents
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,	1.0f, 0.0f, 0.0f,

			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,	0.0f, 1.0f, 0.0f,
			-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,	0.0f, 1.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
			-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,	0.0f, 1.0f, 0.0f,
			-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,	0.0f, 1.0f, 0.0f,
			-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,	0.0f, 1.0f, 0.0f,

			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,	0.0f, -1.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,	0.0f, -1.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,	0.0f, -1.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,	0.0f, -1.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,	0.0f, -1.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,	0.0f, -1.0f, 0.0f,

			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,	1.0f, 0.0f, 0.0f,

			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,	1.0f, 0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,	1.0f, 0.0f, 0.0f,
			-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,	1.0f, 0.0f, 0.0f 
		};
		// configure cube vao and vbo
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);

		glBindVertexArray(cubeVAO);

		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		
	}

	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}

// process all input
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);   

	/*if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (bumpiness + 0.1f <= 1)
			bumpiness += 0.1f;
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (bumpiness - 0.1f >= 0)
			bumpiness -= 0.1f;
	}*/

	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		glDisable(GL_MULTISAMPLE);
	}

	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		glEnable(GL_MULTISAMPLE);
	}

	/*if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
		showGrid = !showGrid;
	}*/
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) {
		if (bumpiness + 0.1f <= 1)
			bumpiness += 0.1f;
	}
	if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) {
		if (bumpiness - 0.1f >= 0)
			bumpiness -= 0.1f;
	}

	if (key == GLFW_KEY_ENTER && action == GLFW_PRESS) {
		showGrid = !showGrid;
	}
}

//  window size
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// calculate index for points, modulo for array reset
int calcCorrectIndex(int index) {
	index++;
	int mod = index % 18;
	if (mod == 0) {
		return 1;
	}
	return mod;
}

//calculate tangents p1 and p2 as helper values
std::vector<glm::vec3> calcTangents(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
	float t = 0.0f;
	float b = 0.0f;
	float c = 0.0f;

	std::vector<glm::vec3> results(2);

	float coef1 = ((1 - t) * (1 + b) * (1 + c)) / 2;
	float coef2 = ((1 - t) * (1 - b) * (1 - c)) / 2;
	float coef3 = ((1 - t) * (1 + b) * (1 - c)) / 2;
	float coef4 = ((1 - t) * (1 - b) * (1 + c)) / 2;

	glm::vec3 tang1 = coef1 * (p1 - p0) + coef2 * (p2 - p1);
	glm::vec3 tang2 = coef3 * (p2 - p1) + coef4 * (p3 - p2);
	results[0] = tang1;
	results[1] = tang2;
	return results;
}

//calculate the point between p0 and p1 based on the t value
glm::vec3 calcPoint(float t, glm::vec3 p0, glm::vec3 p1, glm::vec3 tang1, glm::vec3 tang2) {
	float t2 = t * t;
	float t3 = t * t * t;

	float h00 = 2 * t2 - 3 * t2 + 1;
	float h10 = t3 - 2 * t2 + t;
	float h01 = -2 * t3 + 3 * t3;
	float h11 = t3 - t2;

	return h00 * p0 + h10 * tang1 + h01 * p1 + h11 * tang2;
}

// utility function for loading a 2D texture from file
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); 
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}