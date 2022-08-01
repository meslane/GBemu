#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "shader.h"
#include "cpu.h"

#define WIDTH 160
#define HEIGHT 144

struct Pixel {
	uint8_t r;
	uint8_t g;
	uint8_t b;
}pixel;

union screen { //monochrome 8 bit depth
	Pixel square[HEIGHT][WIDTH];
	Pixel flat[WIDTH * HEIGHT];
};

//Pixel colors[4] = { {15, 56, 15}, {48, 98, 48}, {139, 172, 15}, {155, 188, 15} };
//Pixel colors[4] = { {0,19,26}, {31,89,74}, {108,166,108}, {216,247,215} };
Pixel colors[4] = { {0,0,0}, {85,85,85}, {170,170,170}, {255,255,255} };

static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, GLFW_TRUE);
	}
}

static void sizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

int main(void) {
	glfwInit();

	/* create window */
	GLFWwindow* window = glfwCreateWindow(4*WIDTH, 4*HEIGHT, "GameBoy", NULL, NULL);
	if (!window) {
		std::cout << "ERROR: failed to create GLFW window" << std::endl;
		glfwTerminate();
		return 1;
	}

	glfwSetKeyCallback(window, keyCallback);
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
	glfwSetFramebufferSizeCallback(window, sizeCallback);

	glfwMakeContextCurrent(window);
	gladLoadGL();
	glfwSwapInterval(1);

	/* create shaders */
	GLuint vShader = createShaderFromFile(GL_VERTEX_SHADER, "Shaders/shader.vert");
	GLuint fShader = createShaderFromFile(GL_FRAGMENT_SHADER, "Shaders/shader.frag");

	const GLuint shaderList[] = { vShader, fShader };
	GLuint shaderProgram = createShaderProgram(shaderList, 2);

	glDeleteShader(vShader);
	glDeleteShader(fShader);

	/* define texture dimensions and VBO/VAO */
	float vertices[] = {
		// positions                  // texture coords
		 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,   // top right
		 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,   // bottom right
		-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,   // bottom left
		-1.0f,  1.0f, 0.0f, 0.0f, 1.0f    // top left 
	};
	unsigned int indices[] = {
		0, 1, 3, // first triangle
		1, 2, 3  // second triangle
	};

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL); //position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float))); //texture
	glEnableVertexAttribArray(1);

	/* texture settings */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	screen *display = new screen;
	memset(display->flat, 0, sizeof(display->flat));

	for (int i = 0; i < HEIGHT; i++) {
		for (int j = 0; j < WIDTH; j++) {
			display->square[i][j] = colors[(i+j) % 4];
		}
	}

	uint8_t memory[65536] = { 0 };
	memory[0] = 0x06;
	memory[1] = 0xFF;
	memory[2] = 0x48;
	gbcpu* gb = new gbcpu(memory);

	gb->tick();
	gb->registerDump();
	gb->tick();
	gb->registerDump();
	gb->tick();
	gb->registerDump();
	gb->tick();
	gb->registerDump();

	delete gb;

	/* main loop */
	while (!glfwWindowShouldClose(window)) {

		/* update texture */
		glTexImage2D(GL_TEXTURE_2D, 0, 3, WIDTH, HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, display->flat);

		/* draw triangles */
		glUseProgram(shaderProgram);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, indices);

		/* this code should go last */
		glfwPollEvents();
		glfwSwapBuffers(window);
	}

	delete display;
	return 0;
}