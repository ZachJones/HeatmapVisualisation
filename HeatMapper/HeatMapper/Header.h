#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Window dimensions
const GLuint WIDTH = 1600, HEIGHT = 1200;

GLuint shaderProgram;
GLint vertexColorLocation;

const int playerNum = 8; //Max possible players
const int posNum = 50000; //22150
static glm::vec3 playerPositions[playerNum][posNum];

const int SqPer = 200; //200
const int MaxBoxes = SqPer * SqPer;

GLuint VBO[MaxBoxes], VAO[MaxBoxes], EBO[MaxBoxes];
const GLint indices[] =
{
	0, 1, 3,  // First Triangle
	1, 2, 3   // Second Triangle
};

int hitCount[SqPer][SqPer];
bool entered[playerNum][SqPer*SqPer];

// Camera
float camX = 0.0f;
float camY = 0.0f;
GLuint camWidth = WIDTH;
GLuint camHeight = HEIGHT;

int viewMode = 0;
int maxPlayers;

int p = 1;