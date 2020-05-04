/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_ivan.iglesias_ 1
Author: Ivan Iglesias, ivan.iglesias, 540000716
Creation date: 1/20/2020
----------------------------------------------------------------------------------------------------------*/
#include <GL/gl3w.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include "raytracer.h"
#include "parser.h"

using namespace std;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_buttom_callback(GLFWwindow* window, int button, int action, int mods);


GLFWwindow* window;

int width, height;

void WindowCreation()
{
	// glfw window creation
	window = glfwCreateWindow(width, height, "Framework", NULL, NULL);
	g_raytracer.window = window;
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetMouseButtonCallback(window, mouse_buttom_callback);

}
void InitGlfw()
{
	// glfw: initialize and configure
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	WindowCreation();
}
void LoadOpenGL()
{
	// glad: load all OpenGL function pointers	
	if (gl3wInit())
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
	}
}



int main(int argc, char* argv[])
{
	
	string fileName;
	float mutations = 1.0f;
	float threadUse = 1.0f;
	int depth = 1;


	if (argc < 2)
	{
		cout << "Not size entered" << endl;
		width = 500;
		height = 500;
	}
	else
	{
		string width_text = argv[1];
		string height_text;

		if (argc < 3)
			height_text = width_text;
		else
			height_text = argv[2];

		width = stoi(width_text);
		height = stoi(height_text);

		if (argc == 4)
			mutations = stof(argv[3]);	

		if (argc == 5)
		{
			mutations = stof(argv[3]);
			threadUse = stof(argv[4]);
		}

		if (argc == 6)
		{
			mutations = stof(argv[3]);
			threadUse = stof(argv[4]);
			depth = stoi(argv[5]);
		}

		if (argc == 7)
		{
			mutations = stof(argv[3]);
			threadUse = stof(argv[4]);
			depth = stoi(argv[5]);
			fileName = argv[6];
		}

	}

	InitGlfw();
	LoadOpenGL();		

	//store the arguments of the image size
	g_raytracer.setWidthHeight(width, height);
	//num samples
	g_raytracer.MutationsMLT = mutations;
	g_raytracer.ThreadsCount = threadUse;
	g_raytracer.DepthMLT = depth;

	//parse file
	if(fileName.size())
		Parser::readFileObjects(fileName);
	else
		Parser::readFileObjects("../scenes/CornellBox2.txt");

	g_raytracer.renderScene();

	glfwSwapBuffers(window);
	glfwPollEvents();

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window_framebuffer, int width, int height)
{
	
}

void mouse_buttom_callback(GLFWwindow* window_mouse, int button, int action, int mods)
{



}