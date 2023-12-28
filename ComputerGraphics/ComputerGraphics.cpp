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
void setMaterialValues(glm::vec3, glm::vec3, glm::vec3 = glm::vec3(0.0f, 0.0f, 0.0f), float = 0.2f * 128);

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;
GLuint gVertexArrayObjectCube;

Shader shader;

const glm::vec3 eyes = glm::vec3(4.0f, 2.0f, 13.0f);

const glm::vec3 ceilingLightPosition = glm::vec3(5.5f, 5.0f, 8.0f);
const glm::vec3 nightLampLightPosition = glm::vec3(0.7f, 1.1f, 9.0f);

const glm::vec3 nightLampLightDirection = glm::vec3(0.3f, -1.0f, -0.8f);

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

	std::cout << "Press W for moving forward" << std::endl;
	std::cout << "Press A for moving left" << std::endl;
	std::cout << "Press S for moving backwards" << std::endl;
	std::cout << "Press D for moving right" << std::endl;
	std::cout << std::endl;
	std::cout << "Press Q to reset the camera" << std::endl;
	std::cout << std::endl;
	std::cout << "Press 1 for ceiling lamp" << std::endl;
	std::cout << "Press 2 for night stand lamp" << std::endl;
	std::cout << std::endl;
	std::cout << "Use mouse scroll to zoom in and out" << std::endl;
	std::cout << "Use mouse movement to change the view angle" << std::endl;

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
		camera.ProcessKeyboard(LEFT, deltaTime);
		break;

	case SDLK_d:
		camera.ProcessKeyboard(RIGHT, deltaTime);
		break;

	case SDLK_w:
		camera.ProcessKeyboard(FORWARD, deltaTime);
		break;

	case SDLK_s:
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

		shader.setBool("nightLampStatus", nightLampStatus);
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

		gWindow = SDL_CreateWindow("3D room", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

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

	shader.setVec3("nightLampLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
	shader.setVec3("nightLampLight.position", nightLampLightPosition);
	shader.setVec3("nightLampLightDirection", nightLampLightDirection);

	shader.setFloat("nightLampLightCutOff", glm::cos(glm::radians(35.0f)));

	shader.setFloat("nightLampLightOuterCutOff", glm::cos(glm::radians(40.0f)));

	gVertexArrayObjectCube = createCube();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_DEPTH_TEST);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	return success;
}

void close()
{
	glDeleteProgram(shader.ID);

	glDeleteVertexArrays(1, &gVertexArrayObjectCube);

	SDL_GL_DeleteContext(gContext);

	SDL_DestroyWindow(gWindow);
	gWindow = NULL;

	SDL_Quit();
}

void render()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), 16.0f/9.0f, 2.0f, 1000.0f);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 model = glm::mat4(1);

	glm::mat3 normalMat = glm::transpose(glm::inverse(model));

	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	shader.setMat3("normalMat", normalMat);

	drawRoom();

	drawCeilingLight();

	drawBed();

	drawWardrobe();

	drawNightStand();

	drawNightStandLamp();

    drawShelfs();

	drawMirrorTable();

	//std::cout << glm::to_string(camera.Position) << std::endl;

}

void drawRoom()
{
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//floor
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(5.0f, 0.1f, 7.0f));
	model = glm::translate(model, glm::vec3(-1.0f, -5.0f, 0.0f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.25f, 0.05f, 0.0f);
	diffuse = glm::vec3(0.5f, 0.1f, 0.0f);
	setMaterialValues(ambient, diffuse);

	drawCube();

	//front wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, -1.0f, 2.2f));
	model = glm::scale(model, glm::vec3(5.0f, 2.0f, 0.1f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	ambient = glm::vec3(0.5f, 0.4f, 0.35f);
	diffuse = glm::vec3(1.0f, 0.8f, 0.7f);
	setMaterialValues(ambient, diffuse);

	drawCube();

	//Same material values for the next couple elements of the room

	//right wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(9.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f, 2.0f, 5.5f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();

	//left wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-4.5f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 2.0f, 5.5f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();

	//back wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, -1.0f, 16.0f));
	model = glm::scale(model, glm::vec3(5.0f, 2.0f, 0.1f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();


	//ceiling
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2.0f, 5.1f, 0.0f));
	model = glm::scale(model, glm::vec3(5.0f, 0.1f, 7.0f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.5f, 0.45f, 0.4f);
	diffuse = glm::vec3(1.0f, 0.9f, 0.8f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//carpet
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(3.5f, -0.2f, 7.0f));
	model = glm::scale(model, glm::vec3(1.3f, 0.01f, 1.7f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.20f, 0.05f, 0.0f);
	diffuse = glm::vec3(0.4f, 0.1f, 0.0f);

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

	ambient = glm::vec3(0.25f, 0.1f, 0.1f);
	diffuse = glm::vec3(0.5f, 0.2f, 0.2f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//body
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(1.0f, 0.2f, 0.9f));
	model = glm::translate(model, glm::vec3(0.0f, -0.5f, 6.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.412f, 0.353f, 0.2745f);
	diffuse = glm::vec3(0.824f, 0.706f, 0.549f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//right pillow
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.5f, 0.5f, 6.0f));
	model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.28f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.3135f, 0.161f, 0.088f);
	diffuse = glm::vec3(0.627f, 0.322f, 0.176f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//left pillow
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.5f, 0.5f, 7.2f));
	model = glm::rotate(model, glm::radians(22.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.28f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();

	//blanket
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.4f, 0.45f, 5.5f));
	model = glm::scale(model, glm::vec3(0.5f, 0.05f, 0.95f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//blanket left side
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.4f, -0.3f, 8.2f));
	model = glm::scale(model, glm::vec3(0.5f, 0.25f, 0.05f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//blanket right side
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.4f, -0.3f, 5.5f));
	model = glm::scale(model, glm::vec3(0.5f, 0.25f, 0.05f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();
}

void drawWardrobe() {
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//wardrobe body
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.5f, 0.0f, 3.6f));
	model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.25f, 0.1f, 0.1f);
	diffuse = glm::vec3(0.5f, 0.2f, 0.2f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//vertical stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.5f, 1.0f, 5.11f));
	model = glm::scale(model, glm::vec3(0.5f, 0.01f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.1f, 0.05f, 0.05f);
	diffuse = glm::vec3(0.2f, 0.1f, 0.1f);

	setMaterialValues(ambient, diffuse);
	//Same material values for the next couple elements of the wardrobe

	drawCube();

	//vertical stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.5f, 0.5f, 5.11f));
	model = glm::scale(model, glm::vec3(0.5f, 0.01f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//vertical stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.5f, 0.0f, 5.11f));
	model = glm::scale(model, glm::vec3(0.5f, 0.01f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();

	//right side horizontal stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(8.0f, 0.0f, 5.11f));
	model = glm::scale(model, glm::vec3(0.01f, 1.0f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();

	//left side horizontal stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(7.25f, 1.0f, 5.11f));
	model = glm::scale(model, glm::vec3(0.01f, 0.67f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();

	//left side horizontal stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(6.5f, 0.0f, 5.11f));
	model = glm::scale(model, glm::vec3(0.01f, 1.0f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();

	//right handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(7.5f, 1.4f, 5.11f));
	model = glm::scale(model, glm::vec3(0.02f, 0.18f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();

	//left handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(7.0f, 1.4f, 5.11f));
	model = glm::scale(model, glm::vec3(0.02f, 0.18f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	drawCube();

	//drawer handle 1
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(7.0f, 0.7f, 5.11f));
	model = glm::scale(model, glm::vec3(0.16f, 0.02f, 0.01f));
	model = generateDefaultModelMatrixCube(model);
	shader.setMat4("model", model);
	drawCube();


	//drawer handle 2
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(7.0f, 0.25f, 5.11f));
	model = glm::scale(model, glm::vec3(0.16f, 0.02f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

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

	ambient = glm::vec3(0.1f, 0.05f, 0.05f);
	diffuse = glm::vec3(0.2f, 0.1f, 0.1f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	// drawer
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.88f, 0.0f, 8.8f));
	model = glm::scale(model, glm::vec3(0.0001f, 0.11f, 0.18f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.15f, 0.1f, 0.1f);
	diffuse = glm::vec3(0.3f, 0.2f, 0.2f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//drawer's knob
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.9f, 0.15f, 9.05f));
	model = glm::scale(model, glm::vec3(0.01f, 0.02f, 0.02f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.15f, 0.05f, 0.0f);
	diffuse = glm::vec3(0.3f, 0.1f, 0.0f);

	setMaterialValues(ambient, diffuse);


	drawCube();
}

void drawShelfs() {
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//middle shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.5f, 1.9f, 3.0f));
	model = glm::scale(model, glm::vec3(0.4f, 0.03f, 0.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.1f, 0.05f, 0.05f);
	diffuse = glm::vec3(0.2f, 0.1f, 0.1f);

	setMaterialValues(ambient, diffuse);
	//Same material values for the next couple elements of shelfs

	drawCube();

	//top shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.0f, 2.3f, 3.0f));
	model = glm::scale(model, glm::vec3(0.4f, 0.03f, 0.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//bottom shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.0f, 1.5f, 3.0f));
	model = glm::scale(model, glm::vec3(0.4f, 0.03f, 0.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();


	//item 1 on middle shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.5f, 1.9f, 3.0f));
	model = glm::scale(model, glm::vec3(0.05f, 0.16f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.4315f, 0.039f, 0.1175f);
	diffuse = glm::vec3(0.863f, 0.078f, 0.235f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//item 2 on middle shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.8f, 1.9f, 3.0f));
	model = glm::scale(model, glm::vec3(0.05f, 0.12f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.39f, 0.041f, 0.261f);
	diffuse = glm::vec3(0.780f, 0.082f, 0.522f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//item 1 on top shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.11f, 2.3f, 3.1f));
	model = glm::scale(model, glm::vec3(0.16f, 0.1f, 0.1f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.502f, 0.502f, 0.0f);
	diffuse = glm::vec3(0.416f, 0.353f, 0.804f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//item 2 on top shelf lower part
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(2.0f, 2.31f, 3.0f));
	model = glm::scale(model, glm::vec3(0.04f, 0.06f, 0.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.39f, 0.041f, 0.261f);
	diffuse = glm::vec3(0.780f, 0.082f, 0.522f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//item 2 on top shelf upper part
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(2.01f, 2.46f, 3.0f));
	model = glm::scale(model, glm::vec3(0.01f, 0.05f, 0.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.2645f, 0.404f, 0.49f);
	diffuse = glm::vec3(0.529f, 0.808f, 0.98f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//item 1 on the bottom shelf
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.8f, 1.5f, 3.0f));
	model = glm::scale(model, glm::vec3(0.09f, 0.1f, 0.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.349f, 0.65f, 0.065f);
	diffuse = glm::vec3(0.698f, 0.133f, 0.133f);

	setMaterialValues(ambient, diffuse);

	drawCube();
}

void drawMirrorTable() {
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//left drawer
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.5f, 0.0f, 4.6f));
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.2725f, 0.1355f, 0.0375f);
	diffuse = glm::vec3(0.545f, 0.271f, 0.075f);

	setMaterialValues(ambient, diffuse);
	//Same material values for the next couple elements of the mirrorTable

	drawCube();

	//right drawer
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.6f, 0.0f, 4.6f));
	model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//middle drawer
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.5f, 0.6f, 4.6f));
	model = glm::scale(model, glm::vec3(0.57f, 0.1f, 0.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//middle drawer bottom stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.5f, 0.6f, 5.2f));
	model = glm::scale(model, glm::vec3(0.57f, 0.01f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.1f, 0.05f, 0.05f);
	diffuse = glm::vec3(0.2f, 0.1f, 0.1f);

	setMaterialValues(ambient, diffuse);
	//Same material values for the next couple elements of the mirrorTable
	drawCube();

	//middle drawer top stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.5f, 0.9f, 5.2f));
	model = glm::scale(model, glm::vec3(0.57f, 0.01f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//middle drawer handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.1f, 0.75f, 5.2f));
	model = glm::scale(model, glm::vec3(0.16f, 0.02f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//left body handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.0f, 0.1f, 5.2f));
	model = glm::scale(model, glm::vec3(0.02f, 0.13f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//right body handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.7f, 0.1f, 5.2f));
	model = glm::scale(model, glm::vec3(0.02f, 0.13f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//mirror left stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.77f, 0.9f, 4.71f));
	model = glm::scale(model, glm::vec3(0.019f, 0.49f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();


	//mirror right stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.85f, 0.9f, 4.71f));
	model = glm::scale(model, glm::vec3(0.019f, 0.49f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	
	drawCube();

	//mirror bottom stripe 
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.77f, 0.9f, 4.71f));
	model = glm::scale(model, glm::vec3(0.36f, 0.019f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	//mirror top stripe
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.77f, 2.35f, 4.71f));
	model = glm::scale(model, glm::vec3(0.379f, 0.019f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	drawCube();

	// mirror
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.8f, 0.9f, 4.7f));
	model = glm::scale(model, glm::vec3(0.36f, 0.5f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.345f, 0.439f, 0.451f);
	diffuse = glm::vec3(0.690f, 0.878f, 0.902f);

	float shinines = 50.0f;
	setMaterialValues(ambient, diffuse,glm::vec3(0,0,0), shinines);

	drawCube();
}

void drawNightStandLamp() {
	glm::mat4 model;
	glm::vec3 ambient;
	glm::vec3 diffuse;

	//lamp base
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.6f, 0.5f, 8.95f));
	model = glm::scale(model, glm::vec3(0.07f, 0.02f, 0.07f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.0f, 0.0f, 0.5f);
	diffuse = glm::vec3(0.0f, 0.0f, 1.0f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//stand
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.7f, 0.35f, 9.050f));
	model = glm::scale(model, glm::vec3(0.01f, 0.2f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.8f, 0.8f, 0.8f);
	diffuse = glm::vec3(1.0f, 1.0f, 1.0f);

	setMaterialValues(ambient, diffuse);

	drawCube();

	//shade
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.6f, 0.9f, 8.9f));
	model = glm::scale(model, glm::vec3(0.08f, 0.09f, 0.08f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);

	ambient = glm::vec3(0.0f, 0.0f, 0.2725f);
	diffuse = glm::vec3(0.0f, 0.0f, 0.545f);
	glm::vec3 emission = glm::vec3(0.0f, 0.0f, 0.0f);

	if (nightLampStatus) {
		emission = glm::vec3(0.0f, 0.0f, 0.5f);
	}

	setMaterialValues(ambient, diffuse, emission);

	drawCube();

}

void drawCeilingLight() {
	glm::mat4 model = glm::mat4(1);

	model = glm::translate(model, glm::vec3(5.0f, 5.0f, 7.5f));
	model = glm::scale(model, glm::vec3(0.3f, 0.03f, 0.3f));
	model = generateDefaultModelMatrixCube(model);

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

void setMaterialValues(glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 emission, float shininess) {

	shader.setVec3("fragMaterial.ambient", ambient);
	shader.setVec3("fragMaterial.diffuse", diffuse);
	shader.setVec3("fragMaterial.emission", emission);
	shader.setVec3("fragMaterial.specular", 1.0f, 1.0f, 1.0f);
	shader.setFloat("fragMaterial.shininess", shininess);

	shader.setFloat("fragMaterial.ka", 1.0f);
	shader.setFloat("fragMaterial.kd", 1.0f);
	shader.setFloat("fragMaterial.ks", 1.0f);

}