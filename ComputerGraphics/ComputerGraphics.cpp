#include <iostream>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include "glm/ext.hpp"
#include "glm/gtx/string_cast.hpp"

#define STD_IMAGE_IMPLEMENTATION
#include "stb_image.h"


/*
 Function definition
*/

//general function
bool init();
bool initGL();
void render();
void close();
void handleKeyDown(const SDL_KeyboardEvent&);
void handleMouseMotion(const SDL_MouseMotionEvent&);
void handleMouseWheel(const SDL_MouseWheelEvent&);

//element functions
void drawRoom();
void drawBed();
void drawWardrobe();
void drawNightStand();
void drawShelfs();
void drawMirrorTable();

void drawNightStandLamp();
void drawCeilingLight();

//objects function
GLuint createCube();
void drawCube();

//helper functions
glm::mat4 generateDefaultModelMatrixCube(glm::mat4);
void setMaterialValues(glm::vec3, glm::vec3, glm::vec3 = glm::vec3(0.0f,0.0f,0.0f));

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;
GLuint gVertexArrayObjectPyramid;
GLuint gVertexArrayObjectCube;

Shader shader;

const glm::vec3 eyes = glm::vec3(4.0f, 2.0f, 13.5f);

const glm::vec3 ceilingLightPosition = glm::vec3(5.5f, 5.0f, 7.0f);
const glm::vec3 nighStandLampLightPosition = glm::vec3(0.7f, 1.1f, 9.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;

float lastX = -1;
float lastY = -1;
bool firstMouse = true;

bool ceilingLampStatus = false;
bool nightLampStatus = false;

Camera camera(eyes);

int main(int argc, char* args[])
{
	init();
	SDL_Event event;
	bool quit = false;
	while (!quit)
	{

		float currentFrame = SDL_GetTicks() / 1000.0f;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		while (SDL_PollEvent(&event) != 0)
		{
			if (event.type == SDL_QUIT)
			{
				quit = true;
			}

			switch (event.type)
			{
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				else
				{
					handleKeyDown(event.key);
				}
				break;
			case SDL_MOUSEMOTION:
				handleMouseMotion(event.motion);
				break;
			case SDL_MOUSEWHEEL:
				handleMouseWheel(event.wheel);
				break;
			}

		}

		render();

		SDL_GL_SwapWindow(gWindow);
	}

	close();

	return 0;
}


void handleKeyDown(const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
	case SDLK_a:
		//eyes.x -= 1.0f;
		camera.ProcessKeyboard(LEFT, deltaTime);
		break;

	case SDLK_d:
		//eyes.x += 1.0f;
		camera.ProcessKeyboard(RIGHT, deltaTime);
		break;

	case SDLK_w:
		//eyes.z -= 1.0f;
		camera.ProcessKeyboard(FORWARD, deltaTime);
		break;

	case SDLK_s:
		//eyes.z += 1.0f;
		camera.ProcessKeyboard(BACKWARD, deltaTime);
		break;

	case SDLK_q:
		camera = Camera(eyes);
		break;

	case SDLK_1:
		if (ceilingLampStatus)
		{
			ceilingLampStatus = false;
		}
		else {
			ceilingLampStatus = true;
		}

		shader.setBool("ceilingLampStatus", ceilingLampStatus);
		break;


	case SDLK_2:
		if (nightLampStatus)
		{
			nightLampStatus = false;
		}
		else {
			nightLampStatus = true;
		}

		shader.setBool("nightLightStatus", nightLampStatus);
		break;

	}
}

void handleMouseMotion(const SDL_MouseMotionEvent& motion) {
	if (firstMouse)
	{
		lastX = motion.x;
		lastY = motion.y;
		firstMouse = false;
	}
	else
	{
		camera.ProcessMouseMovement(motion.x - lastX, lastY - motion.y);
		lastX = motion.x;
		lastY = motion.y;
	}
}

void handleMouseWheel(const SDL_MouseWheelEvent& wheel) {
	camera.ProcessMouseScroll(wheel.y);
}


bool init()
{
	bool success = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
		success = false;
	}
	else
	{
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

		gWindow = SDL_CreateWindow("3D room", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 800, 600, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

		if (gWindow == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
			success = false;
		}
		else
		{
			gContext = SDL_GL_CreateContext(gWindow);
			if (gContext == NULL)
			{
				printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
				success = false;
			}
			else
			{
				//Use Vsync
				if (SDL_GL_SetSwapInterval(1) < 0)
				{
					printf("Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError());
				}

				//Initialize OpenGL
				if (!initGL())
				{
					printf("Unable to initialize OpenGL!\n");
					success = false;
				}
			}
		}

	}

	return success;
}


bool initGL()
{
	bool success = true;
	GLenum error = GL_NO_ERROR;

	glewInit();

	error = glGetError();

	if (error != GL_NO_ERROR)
	{
		printf("Error initializing OpenGL! %s\n", gluErrorString(error));
		success = false;
	}

	glClearColor(0, 0, 0, 1);

	shader.Load("./Shaders/vertex.vert", "./Shaders/fragment.frag");
	shader.use();

	shader.setVec3("ceilingLampLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setVec3("ceilingLampLight.position", ceilingLightPosition);

	shader.setVec3("nightLampLight.diffuse", glm::vec3(1, 1, 1));
	shader.setVec3("nightLampLight.position", nighStandLampLightPosition);

	//gVertexArrayObjectPyramid = createPyramid();

	gVertexArrayObjectCube = createCube();

	/*glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);*/


	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	return success;
}

void close()
{
	glDeleteProgram(shader.ID);

	glDeleteVertexArrays(1, &gVertexArrayObjectPyramid);

	SDL_GL_DeleteContext(gContext);

	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	SDL_Quit();
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 4.0f / 3.0f, 2.0f, 1000.0f);
	//glm::mat4 view = glm::lookAt(eyes, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1);

	glm::mat3 normalMat = glm::transpose(glm::inverse(model));

	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	shader.setMat3("normalMat", normalMat);

	drawRoom();

	drawBed();

	drawWardrobe();

	drawCeilingLight();

	drawNightStand();

	/*drawNightStandLamp();

	drawShelfs();

	drawMirrorTable();

	drawWindow();*/

}

void drawRoom()
{
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//floor
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(5, 0.1, 7));
	model = glm::translate(model, glm::vec3(-1, -5, 0));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.412, 0.141, 0.051,1));
	ambient = glm::vec3(0.25, 0.05, 0);
	diffuse = glm::vec3(0.5, 0.1, 0.0);
	setMaterialValues(ambient, diffuse);

	drawCube();

	//front wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, -1.0f, 0.5f));
	model = glm::scale(model, glm::vec3(5.0f, 2.0f, 0.1f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	//shader.setVec4("color", glm::vec4(0.949, 0.651, 0.502,1));
	ambient = glm::vec3(0.5, 0.4, 0.35);
	diffuse = glm::vec3(1, 0.8, 0.7);
	setMaterialValues(ambient, diffuse);

	drawCube();

	//Same material values for the next couple elements of the room

	//right wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(9.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f, 2.0f, 5.0f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.949, 0.651, 0.502,1));
	shader.setMat4("model", model);
	drawCube();

	//left wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-4.5f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 2.0f, 5.0f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.949, 0.651, 0.502, 1));
	shader.setMat4("model", model);
	drawCube();

	//back wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, -1.0f, 14.5f));
	model = glm::scale(model, glm::vec3(5.0f, 2.0f, 0.1f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.949, 0.651, 0.502, 1));
	shader.setMat4("model", model);
	drawCube();


	//ceiling
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2, 5.1, 0));
	model = glm::scale(model, glm::vec3(5, 0.1, 7));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.949, 0.651, 0.502, 1));
	ambient = glm::vec3(0.5, 0.45, 0.4);
	diffuse = glm::vec3(1.0, 0.9, 0.8);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//carpet
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(3.5f, -0.2f, 7.0f));
	model = glm::scale(model, glm::vec3(1.3f, 0.01f, 1.7f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.22, 0.173, 0.145, 1.0f));
	ambient = glm::vec3(0.20, 0.05, 0.0);
	diffuse = glm::vec3(0.4, 0.1, 0.0);

	setMaterialValues(ambient, diffuse);

	drawCube();
}

void drawBed()
{
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//headboard
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.1f, 0.5f, 0.9f));
	model = glm::translate(model, glm::vec3(-2.0f, -0.5f, 6.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.502f, 0.38f, 0.149f, 1.0f));
	ambient = glm::vec3(0.25, 0.1, 0.1);
	diffuse = glm::vec3(0.5, 0.2, 0.2);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//body
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(1.0f, 0.2f, 0.9f));
	model = glm::translate(model, glm::vec3(0.0f, -0.5f, 6.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.502f, 0.38f, 0.149f, 1.0f));
	ambient = glm::vec3(0.412, 0.353, 0.2745);
	diffuse = glm::vec3(0.824, 0.706, 0.549);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//right pillow
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.5f, 0.5f, 6.0f));
	model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.28f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.729f, 0.278f, 0.031f, 1.0f)); 
	ambient = glm::vec3(0.3135, 0.161, 0.088);
	diffuse = glm::vec3(0.627, 0.322, 0.176);

	setMaterialValues(ambient, diffuse);
	// Same material values for the next couple elements of the bed

	drawCube();

	//left pillow
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.5f, 0.5f, 7.2f));
	model = glm::rotate(model, glm::radians(22.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.28f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.729f, 0.278f, 0.031f, 1.0f));
	drawCube();

	//blanket
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.4f, 0.45f, 5.5f));
	model = glm::scale(model, glm::vec3(0.5f, 0.05f, 0.95f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.729f, 0.424f, 0.031f, 1.0f));
	drawCube();

	//blanket left side
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.4f, -0.3f, 8.2f));
	model = glm::scale(model, glm::vec3(0.5f, 0.25f, 0.05f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.729f, 0.424f, 0.031f, 1.0f));
	drawCube();

	//blanket right side
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.4f, -0.3f, 5.5f));
	model = glm::scale(model, glm::vec3(0.5f, 0.25f, 0.05f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.729f, 0.424f, 0.031f, 1.0f));
	drawCube();
}

void drawWardrobe() {
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//wardrobe body
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.0f, 0.0f, 3.3f));
	model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.5f, 0.2f, 0.2f, 1.0f));
	ambient = glm::vec3(0.25, 0.1, 0.1);
	diffuse = glm::vec3(0.5, 0.2, 0.2);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//vertical stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.0f, 1.0f, 4.8f));
	model = glm::scale(model, glm::vec3(0.5f, 0.01f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	ambient = glm::vec3(0.1, 0.05, 0.05);
	diffuse = glm::vec3(0.2, 0.1, 0.1);

	setMaterialValues(ambient, diffuse);
	//Same material values for the next couple elements of the wardrobe

	drawCube();

	//vertical stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.0f, 0.5f, 4.8f));
	model = glm::scale(model, glm::vec3(0.5f, 0.01f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//vertical stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.0f, 0.0f, 4.8f));
	model = glm::scale(model, glm::vec3(0.5f, 0.01f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//horizontal stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.5f, 0.0f, 4.8f));
	model = glm::scale(model, glm::vec3(0.01f, 1.0f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//right side horizontal stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.75f, 1.0f, 4.8f));
	model = glm::scale(model, glm::vec3(0.01f, 0.67f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//left side horizontal stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.0f, 0.0f, 4.8f));
	model = glm::scale(model, glm::vec3(0.01f, 1.0f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//right handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.0f, 1.4f, 4.8f));
	model = glm::scale(model, glm::vec3(0.02f, 0.18f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//left handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.5f, 1.4f, 4.8f));
	model = glm::scale(model, glm::vec3(0.02f, 0.18f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//drawer handle 1
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.5f, 0.7f, 4.8f));
	model = glm::scale(model, glm::vec3(0.16f, 0.02f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();


	//drawer handle 2
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.5f, 0.25f, 4.8f));
	model = glm::scale(model, glm::vec3(0.16f, 0.02f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();
}

void drawNightStand() {
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//nightStand main body
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.5f, -0.1f, 8.7f));
	model = glm::scale(model, glm::vec3(0.12f, 0.2f, 0.23f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	//shader.setVec4("color", glm::vec4(0.2, 0.1, 0.1, 1.0));

	ambient = glm::vec3(0.1, 0.05, 0.05);
	diffuse = glm::vec3(0.2, 0.1, 0.1);

	setMaterialValues(ambient, diffuse);

	drawCube();

	// drawer
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.88f, 0.0f, 8.8f));
	model = glm::scale(model, glm::vec3(0.0001f, 0.11f, 0.18f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	//shader.setVec4("color", glm::vec4(0.3, 0.2, 0.2, 1.0));

	ambient = glm::vec3(0.15, 0.1, 0.1);
	diffuse = glm::vec3(0.3, 0.2, 0.2);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//drawer's knob
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.9f, 0.15f, 9.05f));
	model = glm::scale(model, glm::vec3(0.01f, 0.02f, 0.02f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	//shader.setVec4("color", glm::vec4(0.3, 0.1, 0.0, 1.0));

	ambient = glm::vec3(0.15, 0.05, 0.0);
	diffuse = glm::vec3(0.3, 0.1, 0.0);

	setMaterialValues(ambient, diffuse);


	drawCube();
}

void drawShelfs() {
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//middle shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.5, 1.9, 3));
	model = glm::scale(model, glm::vec3(0.4, 0.03, 0.2));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	//top shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1, 2.3, 3));
	model = glm::scale(model, glm::vec3(0.4, 0.03, 0.2));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	//bottom shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1, 1.5, 3));
	model = glm::scale(model, glm::vec3(0.4, 0.03, 0.2));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();


	//item 1 on middle shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.5, 1.9, 3));
	model = glm::scale(model, glm::vec3(0.05, 0.16, 0.01));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.4315, 0.039, 0.1175, 1.0));
	drawCube();

	//item 2 on middle shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.8, 1.9, 3));
	model = glm::scale(model, glm::vec3(0.05, 0.12, 0.01));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.39, 0.041, 0.261, 1.0));
	drawCube();

	//item 1 on top shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.11, 2.3, 3.1));
	model = glm::scale(model, glm::vec3(0.16, 0.1, 0.1));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.502, 0.502, 0.000, 1.0));
	drawCube();

	//item 2 on top shelf lower part
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(2, 2.31, 3));
	model = glm::scale(model, glm::vec3(0.04, 0.06, 0.2));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.39, 0.041, 0.261, 1.0));
	drawCube();

	//item 2 on top shelf upper part
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(2.01, 2.46, 3));
	model = glm::scale(model, glm::vec3(0.01, 0.05, 0.2));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.2645, 0.404, 0.49, 1.0));
	drawCube();

	//item 1 on the bottom shelf	1, 1.5, 3
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.8, 1.5, 3));
	model = glm::scale(model, glm::vec3(0.09, 0.10, 0.2));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.349, 0.65, 0.065, 1.0));
	drawCube();
}

void drawMirrorTable() {
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//left drawer
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.9, 0, 4.6));
	model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.2725, 0.1355, 0.0375, 1.0));

	drawCube();

	//right drawer
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(7, 0, 4.6));
	model = glm::scale(model, glm::vec3(0.2, 0.2, 0.2));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.2725, 0.1355, 0.0375, 1.0));
	drawCube();

	//middle drawer
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.9, 0.6, 4.6));
	model = glm::scale(model, glm::vec3(0.57, 0.1, 0.2));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.2725, 0.1355, 0.0375, 1.0));
	drawCube();

	//middle drawer bottom stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.9, 0.6, 5.2));
	model = glm::scale(model, glm::vec3(0.57, 0.01, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	//middle drawer top stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.9, 0.9, 5.2));
	model = glm::scale(model, glm::vec3(0.57, 0.01, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	//middle drawer handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.5, 0.75, 5.2));
	model = glm::scale(model, glm::vec3(0.16, 0.02, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	//left body handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.4, 0.1, 5.2));
	model = glm::scale(model, glm::vec3(0.02, 0.13, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	//right body handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(7.1, 0.1, 5.2));
	model = glm::scale(model, glm::vec3(0.02, 0.13, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	// mirror
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.2, 0.9, 4.7));
	model = glm::scale(model, glm::vec3(0.36, 0.5, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.345, 0.439, 0.451, 10.0));
	drawCube();

	//mirror left stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.17, 0.9, 4.71));
	model = glm::scale(model, glm::vec3(0.019, 0.49, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	//mirror right stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(7.25, 0.9, 4.71));
	model = glm::scale(model, glm::vec3(0.019, 0.49, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	//mirror bottom stripe 
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.17, 0.9, 4.71));
	model = glm::scale(model, glm::vec3(0.36, 0.019, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

	//mirror top stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.17, 2.35, 4.71));
	model = glm::scale(model, glm::vec3(0.379, 0.019, 0.0001));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.1, 0.05, 0.05, 1.0));
	drawCube();

}

void drawNightStandLamp() {
	glm::mat4 model;

	//lamp base
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.6f, 0.5f, 8.95f));
	model = glm::scale(model, glm::vec3(0.07f, 0.02f, 0.07f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0, 0, 1, 1.0));
	drawCube();

	//stand
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.7f, 0.35f, 9.050f));
	model = glm::scale(model, glm::vec3(0.01f, 0.2f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(1, 1, 1, 1.0));
	drawCube();

	//shade
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.6f, 0.9f, 8.9f));
	model = glm::scale(model, glm::vec3(0.08f, 0.09f, 0.08f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0, 0, 0.545, 1.0));
	drawCube();

}

void drawCeilingLight() {
	glm::mat4 model = glm::mat4(1);

	model = glm::translate(model, glm::vec3(5.0f, 5.0f, 6.5f));
	model = glm::scale(model, glm::vec3(0.3, 0.03, 0.3));
	model = generateDefaultModelMatrixCube(model);

	//shader.setVec4("color", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));

	glm::vec3 ambient = glm::vec3(0.7f, 0.7f, 0.7f);
	glm::vec3 diffuse = glm::vec3(1.0f, 0.843f, 0.0f);
	glm::vec3 emission = glm::vec3(0.0f, 0.0f, 0.0f);

	if (ceilingLampStatus) {
		emission = glm::vec3(1.0f, 1.0f, 1.0f);
	}

	setMaterialValues(ambient, diffuse, emission);

	shader.setMat4("model", model);
	drawCube();
}

GLuint createCube()
{
	//each side of the cube with its own vertices to use different normals
	float vertices[] = {
		//front side
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		//back side
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		//left side
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		//right side
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		//bottom side
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

		//top side
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
	};

	GLuint vertexArrayObject;
	GLuint vertexBufferObject;

	glGenBuffers(1, &vertexBufferObject);
	glGenVertexArrays(1, &vertexArrayObject);

	glBindVertexArray(vertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);


	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	return vertexArrayObject;
}

void drawCube() {
	glBindVertexArray(gVertexArrayObjectCube);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}

glm::mat4 generateDefaultModelMatrixCube(glm::mat4 model)
{
	model = glm::translate(model, glm::vec3(1.5f, 1.5f, 1.5f));
	model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

	return model;
}

void setMaterialValues(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 emission) {

	shader.setVec3("fragMaterial.ambient", ambient);
	shader.setVec3("fragMaterial.diffuse", diffuse);
	shader.setVec3("fragMaterial.emission", emission);
	shader.setVec3("fragMaterial.specular", 1, 1, 1);
	shader.setFloat("fragMaterial.shininess", 0.2f * 128);

	shader.setFloat("fragMaterial.ka", 1.0f);
	shader.setFloat("fragMaterial.kd", 1.0f);
	shader.setFloat("fragMaterial.ks", 1.0f);

}