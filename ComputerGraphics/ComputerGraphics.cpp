#include <iostream>
#include <GL/glew.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <gl/GLU.h>
#include <glm/glm.hpp>
#include "Shader.h"

#define STD_IMAGE_IMPLEMENTATION
#include "stb_image.h"


//function definition

//general function
bool init();
bool initGL();
void render();
void close();
void handleKeyUp(const SDL_KeyboardEvent& key);

//objects function
GLuint createCube();
GLuint createCube1();
void drawCube();
GLuint createPyramid();
void drawPyramid(GLuint);

//element functions
void drawRoom();
void drawBed();
void drawWardrobe();
void drawCeilingLight();
void drawNightLamp();

//helper functions
glm::mat4 generateDefaultModelMatrixCube(glm::mat4);

SDL_Window* gWindow = NULL;
SDL_GLContext gContext;
GLuint gVertexArrayObjectPyramid;
GLuint gVertexArrayObjectCube;

Shader shader;

glm::vec3 eyes = glm::vec3(7.0f, 2.0f, 15.0f);
glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

int main(int argc, char* args[])
{
	init();
	SDL_Event event;
	bool quit = false;
	while (!quit)
	{
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
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					quit = true;
				}
				else
				{
					handleKeyUp(event.key);
				}
				break;
			}

		}

		render();

		SDL_GL_SwapWindow(gWindow);
	}

	close();

	return 0;
}


void handleKeyUp(const SDL_KeyboardEvent& key)
{
	switch (key.keysym.sym)
	{
	case SDLK_a:
		eyes.x -= 1.0f;
		break;

	case SDLK_d:
		eyes.x += 1.0f;
		break;

	case SDLK_w:
		eyes.z -= 1.0f;
		break;

	case SDLK_s:
		eyes.z += 1.0f;
		break;

	case SDLK_q:
		eyes = glm::vec3(7.0f, 2.0f, 15.0f);
		center = glm::vec3(0.0f, 0.0f, 0.0f);
		break;
	}
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

	gVertexArrayObjectPyramid = createPyramid();

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

	glm::mat4 projection = glm::perspective(glm::radians(60.0f), 1.0f, 1.0f, 100.0f);
	glm::mat4 view = glm::lookAt(eyes, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 model = glm::mat4(1);

	glm::mat3 normalMat = glm::transpose(glm::inverse(model));

	shader.setMat4("projection", projection);
	shader.setMat4("view", view);
	shader.setMat3("normalMat", normalMat);

	drawRoom();
	
	drawBed();

	drawWardrobe();

	drawCeilingLight();

}

void drawRoom()
{
	glm::mat4 model;
	
	//floor
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(5, 0.1, 7));
	model = glm::translate(model, glm::vec3(-1, -5, 0));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0, 0, 1,1));
	shader.setMat4("model", model);
	drawCube();

	//front wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.5f, -1.0f, 0.5f));
	model = glm::scale(model, glm::vec3(5.0f, 2.0f, 0.1f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(1, 1, 1,1));
	shader.setMat4("model", model);
	drawCube();

	//right wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(8.0f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(0.2f, 2.0f, 5.0f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(1, 0, 0,1));
	shader.setMat4("model", model);
	drawCube();

	//left wall
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-4.5f, -1.0f, 0.0f));
	model = glm::scale(model, glm::vec3(1.0f, 2.0f, 5.0f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(1, 1, 0, 1));
	shader.setMat4("model", model);
	drawCube();


	//ceiling
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-2, 5.1, 0));
	model = glm::scale(model, glm::vec3(5, 0.1, 7));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0, 1, 0, 1));
	shader.setMat4("model", model);
	drawCube();

	//carpet
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(3.5f, -0.2f, 7.0f));
	model = glm::scale(model, glm::vec3(1.3f, 0.01f, 1.7f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.439f, 0.125f, 0.063f, 1.0f));
	shader.setMat4("model", model);
	drawCube();
}

void drawBed()
{
	glm::mat4 model;

	//headboard
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.1f, 0.5f, 0.9f));
	model = glm::translate(model, glm::vec3(-2.0f, -0.5f, 6.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.502f, 0.38f, 0.149f, 1.0f));
	drawCube();
	
	//body
	model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(1.0f, 0.2f, 0.9f));
	model = glm::translate(model, glm::vec3(0.0f, -0.5f, 6.2f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.502f, 0.38f, 0.149f, 1.0f));
	drawCube();

	//right pillow
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.5f, 0.5f, 6.0f));
	model = glm::rotate(model, glm::radians(20.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.28f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.729f, 0.278f, 0.031f, 1.0f));
	drawCube();

	//left pillow
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(0.5f, 0.5f, 7.2f));
	model = glm::rotate(model, glm::radians(22.0f), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, glm::vec3(0.1f, 0.15f, 0.28f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.729f, 0.278f, 0.031f, 1.0f));
	drawCube();

	//blanket
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.4f, 0.45f, 5.5f));
	model = glm::scale(model, glm::vec3(0.5f, 0.05f, 0.95f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.729f, 0.424f, 0.031f, 1.0f));
	drawCube();

	//blanket left side
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.4f, -0.3f, 8.2f));
	model = glm::scale(model, glm::vec3(0.5f, 0.25f, 0.05f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.729f, 0.424f, 0.031f, 1.0f));
	drawCube();

	//blanket right side TODO
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(1.4f, -0.3f, 5.5f));
	model = glm::scale(model, glm::vec3(0.5f, 0.25f, 0.05f));
	model = generateDefaultModelMatrixCube(model);

	shader.setMat4("model", model);
	shader.setVec4("color", glm::vec4(0.729f, 0.424f, 0.031f, 1.0f));
	drawCube();
}

void drawWardrobe() {
	glm::mat4 model;

	//wardrobe body
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.0f, 0.0f, 3.3f));
	model = glm::scale(model, glm::vec3(0.5f, 1.0f, 0.5f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.5f, 0.2f, 0.2f, 1.0f));
	shader.setMat4("model", model);

	drawCube();

	//vertical stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.0f, 1.0f, 4.8f));
	model = glm::scale(model, glm::vec3(0.5f, 0.01f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);
	
	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
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

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//horizontal stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.5f, 0.0f, 4.8f));
	model = glm::scale(model, glm::vec3(0.01f, 1.0f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//right side horizontal stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.75f, 1.0f, 4.8f));
	model = glm::scale(model, glm::vec3(0.01f, 0.67f, 0.0001f));
	model = generateDefaultModelMatrixCube(model); 

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model); 
	drawCube();

	//left side horizontal stripline
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.0f, 0.0f, 4.8f));
	model = glm::scale(model, glm::vec3(0.01f, 1.0f, 0.0001f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	//right handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.0f, 1.4f, 4.8f));
	model = glm::scale(model, glm::vec3(0.02f, 0.18f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f)); 
	shader.setMat4("model", model);
	drawCube(); 

	//left handle
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.5f, 1.4f, 4.8f));
	model = glm::scale(model, glm::vec3(0.02f, 0.18f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model); 
	drawCube();

	//drawer handle 1
	model = glm::mat4(1); 
	model = glm::translate(model, glm::vec3(4.5f, 0.7f, 4.8f));
	model = glm::scale(model, glm::vec3(0.16f, 0.02f, 0.01f));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f));
	shader.setMat4("model", model);
	drawCube();


	//drawer handle 2
	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(4.5f, 0.25f, 4.8f));
	model = glm::scale(model, glm::vec3(0.16f, 0.02f, 0.01f));
	model = generateDefaultModelMatrixCube(model); 

	shader.setVec4("color", glm::vec4(0.2f, 0.1f, 0.1f, 1.0f)); 
	shader.setMat4("model", model); 
	drawCube();
}

void drawCeilingLight() {
	glm::mat4 model = glm::mat4(1);

	model = glm::translate(model, glm::vec3(5.0f, 5.0f, 7.5f));
	model = glm::scale(model, glm::vec3(0.3, 0.03, 0.3));
	model = generateDefaultModelMatrixCube(model);

	shader.setVec4("color", glm::vec4(0.7f, 0.7f, 0.7f, 1.0f));
	shader.setMat4("model", model);
	drawCube();

	model = glm::mat4(1);
	model = glm::translate(model, glm::vec3(5.0f, 5.0f, 7.5f));
	/*model = glm::scale(model, glm::vec3(0.3, 0.03, 0.3));
	model = generateDefaultModelMatrixCube(model);*/

	shader.setVec4("color", glm::vec4(1,1,1, 1.0f));
	shader.setMat4("model", model);
	drawCube();



}

void drawNightLamp()
{

}

GLuint createPyramid()
{
	float vertices[] =
	{
		-0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, 0.5f,
		0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f - 0.5f,
		0.0f, 0.5f, 0.0f
	};

	GLuint indices[] =
	{
		0, 1, 4,
		1, 2, 4,
		2, 3, 4,
		3, 0, 4
	};

	GLuint vertexArrayObject;
	GLuint vertexBufferObject;
	GLuint elementBufferObject;

	glGenBuffers(1, &vertexBufferObject);
	glGenBuffers(1, &elementBufferObject);
	glGenVertexArrays(1, &vertexArrayObject);

	glBindVertexArray(vertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return vertexArrayObject;
}

void drawPyramid(GLuint vertexArrayObjectId)
{
	glBindVertexArray(vertexArrayObjectId);

	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, (void*)0);
	glBindVertexArray(0);
}


GLuint createCube1() {

	float vertices[] = {
   -0.5f, -0.5f, -0.5f, // Back bottom left
	0.5f, -0.5f, -0.5f, // Back bottom right
	0.5f,  0.5f, -0.5f, // Back top right
   -0.5f,  0.5f, -0.5f, // Back top left

	-0.5f, -0.5f,  0.5f, // Front bottom left
	0.5f, -0.5f,  0.5f, // Front bottom right
	0.5f,  0.5f,  0.5f, // Front top right
	-0.5f,  0.5f,  0.5f, // Front top left
	};

	int indices[] = {
		// Front face
		0, 1, 2,
		2, 3, 0,

		// Back face
		7, 6, 5,
		5, 4, 7,

		// Left face
		4, 0, 3,
		3, 7, 4,

		// Right face
		1, 5, 6,
		6, 2, 1,

		// Top face
		2, 6, 7,
		7, 3, 2,

		// Bottom face
		0, 4, 5,
		5, 1, 0,
	};

	GLuint vertexArrayObject;
	GLuint vertexBufferObject;
	GLuint elementBufferObject;

	glGenBuffers(1, &vertexBufferObject);
	glGenBuffers(1, &elementBufferObject);
	glGenVertexArrays(1, &vertexArrayObject);

	glBindVertexArray(vertexArrayObject);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferObject);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindVertexArray(0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	return vertexArrayObject;

	return 0;
}

GLuint createCube()
{
	//each side of the cube with its own vertices to use different normals
	float vertices[] = {
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

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

GLuint createSphere() {
	const int stacks = 20;
	const int slices = 40;

	return 0;
}

void drawSphere() {

}

glm::mat4 generateDefaultModelMatrixCube(glm:: mat4 model)
{
	model = glm::translate(model, glm::vec3(1.5f, 1.5f, 1.5f));
	model = glm::scale(model, glm::vec3(3.0f, 3.0f, 3.0f));

	return model;
}