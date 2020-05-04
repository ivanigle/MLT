/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_ivan.iglesias_ 1
Author: Ivan Iglesias, ivan.iglesias, 540000716
Creation date: 1/20/2020
----------------------------------------------------------------------------------------------------------*/
#pragma once
#include <glm/gtc/random.hpp>
#include "GLSLProgram.hpp"
#include <vector>
#include "camera.h"
#include "model.h"
#include <GLFW/glfw3.h>

class MLT;
struct threadInfo
{
	int mutations;
	glm::vec3 * image;
	int threadCount;
	int start, end;
	glm::vec3 eyePos;
	int m_height, m_width;
	Camera cam;
	int threadNum;
};
class Raytracer
{
public:

	static Raytracer& getInstance()
	{
		static Raytracer instance; // Guaranteed to be destroyed.
								   // Instantiated on first use.
		return instance;
	}
	Raytracer(Raytracer const&) = delete;
	void operator=(Raytracer const&) = delete;

	Raytracer();

	//-------- Functions -----------------
	//set image sizes
	void setWidthHeight(unsigned width, unsigned height);

	glm::vec3 rayCast(Ray ray, MLT mlt, int d = 5) const;
	glm::vec3 localLightModel(Intersection & minIntersection, Ray & ray, MLT & mlt) const;

	void renderScene();
	void save_ldr(const char *filename, glm::vec3 * finalImage, float exposureAdjust = 1.f);
		
	//-------- Public variables ----------
	
	Camera m_cam;
	std::vector<Shape*> m_renderSphapes;
	
	std::vector<Light> m_lights;

	glm::vec3 m_ambientColor;

	glm::vec3 m_airAtt;

	GLSLProgram texture_program;

	GLFWwindow * window;

	std::vector<unsigned char> colors;


	float MutationsMLT{ 1 };
	float ThreadsCount{ 10 };
	int DepthMLT{ 10 };


	std::vector<bool> threadFinished;

private:

	//Width and height of the output image
	unsigned m_width, m_height;

};

static Raytracer & g_raytracer = Raytracer::getInstance();

