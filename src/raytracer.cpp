/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_ivan.iglesias_ 1
Author: Ivan Iglesias, ivan.iglesias, 540000716
Creation date: 1/20/2020
----------------------------------------------------------------------------------------------------------*/
#include "raytracer.h"
#include <iostream>
#include <fstream> 
#include <algorithm>
#include <windows.h>
#include "MLT.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image/stb_image_write.h>
#include <thread>         // std::thread
#include <mutex>

#define EPSILON 0.000001f
int MAX_DEPTH = 5;


float vertices[] = {
	// positions          // colors           // texture coords
	 1.f,  1.f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 0.0f, // top right
	 1.f, -1.f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 1.0f, // bottom right
	-1.f, -1.f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 1.0f, // bottom left
	-1.f,  1.f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 0.0f  // top left 
};
unsigned int indices[] = {
	0, 1, 3, // first triangle
	1, 2, 3  // second triangle
};

double random_double()
{
	return ((double) rand() / (RAND_MAX));
}

glm::vec3 randSphere(float radius)
{
	glm::vec3 randP;

	randP.x = static_cast<float>(random_double() - 0.5);
	randP.y = static_cast<float>(random_double() - 0.5);
	randP.z = static_cast<float>(random_double() - 0.5);

	randP = glm::normalize(randP);

	float u = static_cast<float>(random_double());
	float c = std::cbrt(u);

	return randP * c * radius;


}

Raytracer::Raytracer()
{
	//set default values
	m_width = 500;
	m_height = 500;
}

void Raytracer::setWidthHeight(unsigned width, unsigned height)
{
	m_width = width;
	m_height = height;
}

// Function to sample a new path for MLT.
// This time I used radiance() (paste training), but anything is fine.
PathSample generate_new_path(Camera &camera, const int width, const int height, MLT &mlt)
{

	int x, y;

	float weight = 4.0;
	
	weight *= width;

	x = static_cast<int>(mlt.NextSample() * width);

	if (x == width)
		x = 0;	
	
	weight *= height;

	y = static_cast<int>(mlt.NextSample() * height);

	if (y == height)
		y = 0;

	int halfWidth = static_cast<int>(width * 0.5f);
	int halfHeight = static_cast<int>(height * 0.5f);


	glm::vec3 viewVec = glm::cross(camera.m_right, camera.m_up);
	viewVec = glm::normalize(viewVec);
	glm::vec3 eyePos = camera.m_position + (viewVec * camera.m_fov);

	glm::vec3 Pxy = camera.m_position + ((x - halfWidth + 0.5f) / halfWidth) * camera.m_right
		- ((y - halfHeight + 0.5f) / halfHeight) *camera.m_up;
	
	glm::vec3 rayVec = Pxy - eyePos;

	Ray ray(eyePos, rayVec);

	glm::vec3 c = g_raytracer.rayCast(ray, mlt, MAX_DEPTH);


	return PathSample(x, y, c, 1.0f / (1.0f / weight));
}
std::mutex g_num_mutex;
void threadWork(threadInfo info)
{
		
	std::vector<glm::vec3> tmp_image;
	tmp_image.resize(info.m_width * info.m_height, glm::vec3(0.0f));

	MLT mlt;

	int seed = rand();
	mlt.xor128_.setSeed(info.threadNum + seed);
		
	float b;
	float p_large = 0.1f;
	int acceptedPaths = 0, rejectPaths = 0;
	PathSample old_path;

	//float sumI = 0.0;
	mlt.large_step = 1;

	while (1) 
	{
		mlt.ResetRandomCoords();
		//Compute new path
		PathSample sample = generate_new_path(info.cam, info.m_width, 
											info.m_height, mlt);			
		mlt.global_time++;
		
		//Clear the stack
		while (!mlt.primary_samples_stack.empty())
			mlt.primary_samples_stack.pop();

		auto value = luminance(sample.Color);
		//Check if valid path
		if (value > 0.0f)
		{				
			b = value;
			p_large = 0.5f;				
			old_path = sample;
			break;
		}			
	}

	const int M = info.mutations;
	int progress = 0;

	for (int i = 0; i < M; i++)
	{

		if ((i + 1) % (M / 10) == 0)
		{
			progress += 10;
			std::cout << "Thread : " << info.threadNum << std::endl;
			std::cout << "Progess: " << progress << "%  ";
			std::cout << "Accept: " << acceptedPaths;
			std::cout << " Reject: " << rejectPaths;
			std::cout << " Rate: " << (100.0f * acceptedPaths / (acceptedPaths + rejectPaths)) << "%" << std::endl;			
		}

		//Check large step
		mlt.large_step = mlt.rand01() < p_large;
		mlt.ResetRandomCoords();

		PathSample newPath = generate_new_path(info.cam, info.m_width, info.m_height, mlt);
		//Compute the acceptance probability
		float a = std::min(1.0f, luminance(newPath.Color) / luminance(old_path.Color));
		//Compute the weights
		const float newPathWeight = (a + mlt.large_step) / (luminance(newPath.Color) / b + p_large) / M;
		const float oldPathWeight = (1.0f - a) / (luminance(old_path.Color) / b + p_large) / M;

		//Store the color on the image
		tmp_image[newPath.y * info.m_width + newPath.x] = tmp_image[newPath.y * info.m_width + newPath.x] + newPath.weight * newPathWeight * newPath.Color;
		tmp_image[old_path.y * info.m_width + old_path.x] = tmp_image[old_path.y * info.m_width + old_path.x] + old_path.weight * oldPathWeight * old_path.Color;


		if (mlt.rand01() < a) // Accept
		{ 
			acceptedPaths++;
			//Set the accepted path as the old one
			old_path = newPath;
			//Check if large step need to be apply
			if (mlt.large_step)
				mlt.large_step_time = mlt.global_time;

			mlt.global_time++;
			
			while (!mlt.primary_samples_stack.empty()) //Stack empty
				mlt.primary_samples_stack.pop();
		}
		else // Rejection
		{ 
			rejectPaths++;

			int idx = mlt.used_rand_coords - 1;

			while (!mlt.primary_samples_stack.empty())
			{
				//Delete from the primary samples the rejected path
				mlt.primary_samples[idx--] = mlt.primary_samples_stack.top();
				mlt.primary_samples_stack.pop();
			}
		}
	}

	g_num_mutex.lock();
	std::cout << "Displaying Thread : " << info.threadNum << std::endl;
	for (int i = 0; i < info.m_width * info.m_height; i++)
	{
		info.image[i] = info.image[i] + glm::clamp(tmp_image[i], 0.0f, 1.0f) / static_cast<float>(info.threadCount);

		//clamp betweem 0 and 1
		auto finalColor = glm::clamp(info.image[i], 0.0f, 1.0f);
		finalColor *= 255.99f;

		g_raytracer.colors[(i * 3)] = static_cast<char>(finalColor.x);
		g_raytracer.colors[(i * 3) + 1] = static_cast<char>(finalColor.y);
		g_raytracer.colors[(i * 3) + 2] = static_cast<char>(finalColor.z);

	}
	g_num_mutex.unlock();	

	g_raytracer.threadFinished[info.threadNum] = true;
}

glm::vec3 Raytracer::rayCast(Ray ray, MLT mlt, int d) const
{

	if (d < -25)
		glm::vec3(0.0);
	//for each sphere check collision 
	Intersection minIntersection;
	minIntersection.m_time = FLT_MAX;
	int objectNum = -1;
	
	

	for (int i = 0; i < m_renderSphapes.size(); i++)
	{
		auto actIntersect = m_renderSphapes[i]->checkIntersection(ray);		

		if (actIntersect.m_time > 0.0f && actIntersect.m_time < minIntersection.m_time)
		{
			minIntersection = actIntersect;
			objectNum = i;
		}
	}

	//Store color for this pixel
	if (minIntersection.m_time != FLT_MAX)
	{

		minIntersection.m_point = minIntersection.m_point + minIntersection.m_normal * EPSILON;

		//offset a bit the initial point
		minIntersection.m_point += minIntersection.m_normal * EPSILON;
		
		float russian_roulette_probability = std::max(minIntersection.m_material.diffuseColor.x, std::max(minIntersection.m_material.diffuseColor.y, minIntersection.m_material.diffuseColor.z));
		if (d < 0)
		{
			if (mlt.NextSample() >= russian_roulette_probability)
				return glm::vec3(0);
		}
		else
		{
			russian_roulette_probability = 1.0; // Did not run russian roulette
		}

		if (m_lights[0].light_id == objectNum)
		{
			if (d == MAX_DEPTH)
				return minIntersection.m_material.emmisionColor;
			else
				return glm::vec3(0);
		}
			   		 	
		
		//direct light
		glm::vec3 finalColor = localLightModel(minIntersection, ray, mlt);
		//glm::vec3 finalColor = glm::vec3(0.0);

		//Reflection and transmission coeff
		float reflectCof = 1.0f;
		float transmCof = 0.0f;

		float reflectLoss = minIntersection.m_material.specularReflection * reflectCof;
		float transmLoss = minIntersection.m_material.specularReflection * transmCof;
		

		float absorb = 1 - reflectLoss - transmLoss;

		finalColor *= absorb;
		
		glm::vec3 w, u, v;
		w = minIntersection.m_normal;
		if (fabs(w.x) > 0.1)
			u = glm::normalize(glm::cross(glm::vec3(0.0, 1.0, 0.0), w));
		else
			u = glm::normalize(glm::cross(glm::vec3(1.0, 0.0, 0.0), w));
		v = glm::cross(w, u);

		
		//Sampling on the hemisphere
		const float r1 = 2 * glm::pi<float>() * mlt.NextSample();
		const float r2 = mlt.NextSample(), r2s = sqrt(r2);
		glm::vec3 dir = glm::normalize((u * cos(r1) * r2s + v * sin(r1) * r2s + w * sqrt(1.0f - r2)));


		return (finalColor + minIntersection.m_material.diffuseColor * rayCast(Ray(minIntersection.m_point, dir), mlt, d - 1)) / static_cast<float>(russian_roulette_probability);		
		

	}
	else
	{
		return glm::vec3(0.0);
	}

	return  glm::vec3(0.0);
}

glm::vec3 Raytracer::localLightModel(Intersection & minIntersection, Ray & ray, MLT & mlt) const
{

	// 	Sampling a point on the light source
	const float r1 = 2.0f * glm::pi<float>() * mlt.NextSample();
	const float r2 = 1.0f - 2.0f * mlt.NextSample();
	const glm::vec3 light_pos = m_lights[0].getPos() + ((m_lights[0].getRadius() + EPSILON) * glm::vec3(sqrt(1.0 - r2 * r2) * cos(r1), sqrt(1.0 - r2 * r2) * sin(r1), r2));

	// 	Calculated from sampled points
	const glm::vec3 light_normal = glm::normalize(light_pos - m_lights[0].getPos());
	const glm::vec3 light_dir = glm::normalize(light_pos - minIntersection.m_point);
	//const double dist2 = glm::length2(light_pos - minIntersection.m_point);
	const float dot0 = glm::dot(minIntersection.m_normal, light_dir);
	const float dot1 = glm::dot(light_normal, -1.0f * light_dir);

	if (dot0 >= 0 && dot1 >= 0) {
		
		
		//for each sphere check collision 
		Intersection actualIntersect;
		actualIntersect.m_time = FLT_MAX;
		int objectNum = -1;
		int i = 0;
		for (; i < m_renderSphapes.size(); i++)
		{
			auto actIntersect = m_renderSphapes[i]->checkIntersection(Ray(minIntersection.m_point, light_dir));
			if (actIntersect.m_time != -1.0f && actIntersect.m_time < actualIntersect.m_time)
			{
				actualIntersect = actIntersect;
				objectNum = i;
			}
		}

		//Store color for this pixel
		if (actualIntersect.m_time == FLT_MAX)
		{
			return glm::vec3(0.0);
		}

		const float dist2 = glm::length2(light_pos - minIntersection.m_point);
		auto value = fabs(sqrt(dist2) - actualIntersect.m_time);


		if (value < 1e-3) {
			
			const float G = dot0 * dot1 / dist2;
			auto color = (minIntersection.m_material.diffuseColor * m_lights[0].getColor()) * (1.0f / glm::pi<float>()) * G / (1.0f / (4.0f *  glm::pi<float>() * pow(m_lights[0].getRadius(), 2.0f)));
		
			return color;
		}
		
	}
	return glm::vec3(0.0);

}



glm::vec3 linearToneMapping(glm::vec3 color, float exposure) {
	color = color * exposure;

	color.x = glm::pow(color.x, 1.f / 2.2f);
	color.y = glm::pow(color.y, 1.f / 2.2f);
	color.z = glm::pow(color.z, 1.f / 2.2f);

	return color;
}


//tigra: time functions
void get_time_str(char * s, clock_t endT, clock_t startT)
{
	int hours, mins, secs;

	float elapsedTime = float((endT - startT) / CLOCKS_PER_SEC);

	hours = (int)floor(elapsedTime / 3600);
	mins = (int)floor((elapsedTime - hours * 3600) / 60);
	secs = (int)floor(elapsedTime - hours * 3600 - mins * 60);

	sprintf(s, "%0.2fs", elapsedTime);

	if (elapsedTime >= 60)
	{
		strcat(s, "(");
		if (hours > 0) sprintf(s, "%s%dh", s, hours);
		if (mins > 0)
		{
			if (hours > 0) strcat(s, " ");
			sprintf(s, "%s%dmin ", s, mins);
		}
		sprintf(s, "%s%02ds)", s, secs);
	}
}

void Raytracer::renderScene()
{
	clock_t startT1, end1;
	startT1 = clock();

	texture_program.compileShaderFromFile("../shader/simple_shader.fs", GLSLShader::GLSLShaderType::FRAGMENT);
	texture_program.compileShaderFromFile("../shader/simple_shader.vs", GLSLShader::GLSLShaderType::VERTEX);
	texture_program.link();


	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	// load and create a texture 
	// -------------------------
	unsigned int texture1;
	// texture 1
	// ---------
	glGenTextures(1, &texture1);
	glBindTexture(GL_TEXTURE_2D, texture1);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


	//texture_program.setUniform("texture1", 0);

	colors.resize(m_width * m_height * 3, 0);	
	
	float halfWidth = static_cast<float>(m_width) * 0.5f;
	float halfHeight = static_cast<float>(m_height) * 0.5f;	

	int total_size = m_width * m_height;
	int ray_num = 0;
	int prev_raynum = 0;
	std::cout << "Mutations : " << MutationsMLT << std::endl;
	std::cout << "Threads in Use : " << ThreadsCount << std::endl;
	std::cout << "Depth Level : " << DepthMLT << std::endl;

	//unsigned * pixelTimes = new bool[m_width * m_height];
	glm::vec3 *finalImage = new glm::vec3[m_width * m_height];
	//Create ray 
	glm::vec3 viewVec = glm::cross(m_cam.m_right, m_cam.m_up);
	viewVec = glm::normalize(glm::vec3(0,1,0));
	glm::vec3 eyePos = m_cam.m_position + (viewVec * m_cam.m_fov);
	
	int threadCount;
	if (ThreadsCount > (std::thread::hardware_concurrency() - 1) || ThreadsCount <= 0)
		threadCount = std::thread::hardware_concurrency() - 1;
	else
		threadCount = static_cast<int>(ThreadsCount);
	
	unsigned mut = static_cast<unsigned>(MutationsMLT);

	MAX_DEPTH = DepthMLT;

	std::vector<std::thread> threadVector;

	threadFinished.resize(threadCount, false);
	srand(static_cast<unsigned>(time(NULL)));
	int sizeThread = total_size / threadCount;
	for (int i = 0; i < threadCount; i++)
	{
		threadInfo info;
		info.cam = m_cam;
		info.end = sizeThread * (i + 1);
		info.start = sizeThread * i;
		info.eyePos = eyePos;
		info.m_height = m_height;
		info.m_width = m_width;
		info.image = finalImage;
		info.threadCount = threadCount;
		info.threadNum = i;
		info.mutations = mut * m_width * m_height;
		threadVector.push_back(std::thread(threadWork, info));
	}


	bool finish = false;

	do
	{
		finish = true;
		for (unsigned i = 0; i < threadFinished.size(); i++)
		{
			if (threadFinished[i] == false)
			{
				finish = false;
			}
			else if(threadVector[i].joinable())
			{
				threadVector[i].join();
			}
		}

		prev_raynum = ray_num;

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, colors.data());
		glGenerateMipmap(GL_TEXTURE_2D);
		// render
   // -----
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture1);

		// render container
		texture_program.use();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();

	} while (!finish);

	end1 = clock();


	char s[200];
	char buf[500];
	get_time_str(s, end1, startT1);

	std::cout << s << std::endl;	
	
	for (unsigned i = 0; i < m_width * m_height; i++)
	{
		auto finalColor = linearToneMapping(finalImage[i], 10.0f);

		finalColor = clamp(finalColor, 0.0f, 1.0f);

		finalColor *= 255.99f;

		g_raytracer.colors[(i * 3)] = static_cast<char>(finalColor.x);
		g_raytracer.colors[(i * 3) + 1] = static_cast<char>(finalColor.y);
		g_raytracer.colors[(i * 3) + 2] = static_cast<char>(finalColor.z);

	}

	if (threadCount > 1)
		sprintf(buf, "%04d_passes%d_%s-%d.png", mut, threadCount, s, static_cast<int>(time(NULL)));
	else
		sprintf(buf, "%04d_%s-%d.png", mut, s, static_cast<int>(time(NULL)));

	stbi_write_png(buf, m_width, m_height, 3, colors.data(), 0);
	prev_raynum = ray_num;

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, m_width, m_height, 0, GL_RGB, GL_UNSIGNED_BYTE, colors.data());
	glGenerateMipmap(GL_TEXTURE_2D);
	// render
// -----
	glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// bind textures on corresponding texture units
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture1);

	// render container
	texture_program.use();
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
	// -------------------------------------------------------------------------------
	glfwSwapBuffers(window);
	glfwPollEvents();



	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

}
