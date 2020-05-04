/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_ivan.iglesias_ 1
Author: Ivan Iglesias, ivan.iglesias, 540000716
Creation date: 1/20/2020
----------------------------------------------------------------------------------------------------------*/
#include "parser.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include "raytracer.h"

glm::vec3 Cen(50, -20, -860);

using namespace std;
void Parser::readFileObjects(std::string filename)
{
	std::ifstream inputFileStream(filename);

	if (!inputFileStream)
	{
		std::cout << "Unable to open file :" << filename << std::endl;
		return;
	}	

	while (inputFileStream)
	{
		std::string line;
		std::getline(inputFileStream, line);

		auto last = line.find_first_of(' ');

		string objectName = line.substr(0, last);
		string lineSpaces = line;

		line.erase(std::remove_if(line.begin(), line.end(), isspace), line.end());

		auto found = line.find_first_of('#');

		if (line.empty() || !found)
			continue;
		

		if (objectName == "SPHERE")
		{
			
			glm::vec3 centerPos = readPoint(line);

			auto endLimit = line.find_last_of(')');

			float radius = std::stof(line.substr(endLimit + 1, line.length()));

			//read material 
			std::getline(inputFileStream, line);

			Material actualMat = readMaterial(line);

			Sphere * newSphere = new Sphere(centerPos, actualMat, radius);
			g_raytracer.m_renderSphapes.push_back(newSphere);

		}
		else if (objectName == "CAMERA")
		{
			auto startLimit = line.find_first_of('(');
			auto endLimit = line.find_first_of(')');

			auto pointText = line.substr(startLimit, (endLimit - startLimit) + 1);

			glm::vec3 camPos = readPoint(pointText);

			//new line
			auto secondLine = line.substr(endLimit + 1, line.size());
			startLimit = secondLine.find_first_of('(');
			endLimit = secondLine.find_first_of(')');
			pointText = secondLine.substr(startLimit, (endLimit - startLimit) + 1);

			glm::vec3 rightVec = readPoint(pointText);

			//new line
			auto thirdLine = secondLine.substr(endLimit + 1, line.size());
			startLimit = thirdLine.find_first_of('(');
			endLimit = thirdLine.find_first_of(')');
			pointText = thirdLine.substr(startLimit, (endLimit - startLimit) + 1);

			glm::vec3 upVec = readPoint(pointText);

			pointText = thirdLine.substr(endLimit + 1, line.size());
			
			float fov = std::stof(pointText);

			g_raytracer.m_cam.m_fov = fov;
			g_raytracer.m_cam.m_position = camPos;
			g_raytracer.m_cam.m_right = rightVec;
			g_raytracer.m_cam.m_up = upVec;			
		}
		else if (objectName == "BOX")
		{

			glm::vec3 pos = readPoint(line);		

			//Read vectors
			std::getline(inputFileStream, line);

			auto startLimit = line.find_first_of('(');
			auto endLimit = line.find_first_of(')');

			auto pointText = line.substr(startLimit, (endLimit - startLimit) + 1);

			glm::vec3 lengthVec = readPoint(pointText);

			//new line
			auto secondLine = line.substr(endLimit + 1, line.size());
			startLimit = secondLine.find_first_of('(');
			endLimit = secondLine.find_first_of(')');
			pointText = secondLine.substr(startLimit, (endLimit - startLimit) + 1);

			glm::vec3 widthVec = readPoint(pointText);

			//new line
			auto thirdLine = secondLine.substr(endLimit + 1, line.size());
			startLimit = thirdLine.find_first_of('(');
			endLimit = thirdLine.find_first_of(')');
			pointText = thirdLine.substr(startLimit, (endLimit - startLimit) + 1);

			glm::vec3 heightVec = readPoint(pointText);

			//read material 
			std::getline(inputFileStream, line);

			Material actualMat = readMaterial(line);

			Box * newBox = new Box(pos, lengthVec, widthVec, heightVec, actualMat);

			g_raytracer.m_renderSphapes.push_back(newBox);

		}
		else if (objectName == "LIGHT")
		{
			auto startLimit = line.find_first_of('(');
			auto endLimit = line.find_first_of(')');

			auto pointText = line.substr(startLimit, (endLimit - startLimit) + 1);

			glm::vec3 lightPos = readPoint(pointText);

			//new line
			auto secondLine = line.substr(endLimit + 1, line.size());
			startLimit = secondLine.find_first_of('(');
			endLimit = secondLine.find_first_of(')');
			pointText = secondLine.substr(startLimit, (endLimit - startLimit) + 1);

			glm::vec3 color = readPoint(pointText);			

			pointText = secondLine.substr(endLimit + 1, line.size());

			float radius = std::stof(pointText);

			Material actualMat;

			actualMat.specularExponent = 0.1f;
			actualMat.specularReflection = 1.0f;
			actualMat.diffuseColor = vec3(0.0f);
			actualMat.emmisionColor = color;


			Sphere * newSphere = new Sphere(lightPos, actualMat, radius);

			
			g_raytracer.m_renderSphapes.push_back(newSphere);
			g_raytracer.m_lights.push_back({lightPos , color, radius, static_cast<int>(g_raytracer.m_renderSphapes.size() - 1)});
			
		}
		else if (objectName == "AMBIENT")
		{
			auto startLimit = line.find_first_of('(');
			auto endLimit = line.find_first_of(')');

			auto pointText = line.substr(startLimit, (endLimit - startLimit) + 1);

			glm::vec3 ambientColor = readPoint(pointText);

			g_raytracer.m_ambientColor = ambientColor;
		}
		else if (objectName == "POLYGON")
		{
			auto endLimit = line.find_first_of('(');
			auto pointText = line.substr(7, 1);

			int numVert = std::stoi(pointText);

			auto startLimit = line.find_first_of('(');

			pointText = line.substr(startLimit, line.length());

			std::vector<glm::vec3> vertex;
			for (int i = 0; i < numVert; i++)
			{
				vertex.push_back(readPoint(pointText));

				auto endLimit = pointText.find_first_of(')');

				pointText = pointText.substr(endLimit + 1, pointText.length());
			}

			//read material 
			std::getline(inputFileStream, line);

			Material actualMat = readMaterial(line);

			Polyg * newPolygon = new Polyg(numVert, vertex, actualMat);

			g_raytracer.m_renderSphapes.push_back(newPolygon);
		
		}
		else if (objectName == "ELLIPSOID")
		{
			auto startLimit = line.find_first_of('(');

			auto pointText = line.substr(startLimit, line.length());

			auto center = readPoint(pointText);

			auto endLimit = pointText.find_first_of(')');

			pointText = pointText.substr(endLimit + 1, pointText.length());

			auto u = readPoint(pointText);

			endLimit = pointText.find_first_of(')');

			pointText = pointText.substr(endLimit + 1, pointText.length());

			auto v = readPoint(pointText);

			endLimit = pointText.find_first_of(')');

			pointText = pointText.substr(endLimit + 1, pointText.length());

			auto w = readPoint(pointText);

			//read material 
			std::getline(inputFileStream, line);

			Material actualMat = readMaterial(line);

			Ellipsoid * newEllipsoid = new Ellipsoid(center, u, v, w, actualMat);

			g_raytracer.m_renderSphapes.push_back(newEllipsoid);
			
		}
		else if (objectName == "AIR")
		{
			auto pos = lineSpaces.find_first_of(' ');

			auto points = lineSpaces.substr(pos + 1, lineSpaces.length());

			pos = points.find_first_of(' ');

			auto magnetic = std::stoi(points.substr(0, pos));

			points = points.substr(pos + 1, points.length());

			pos = points.find_first_of(' ');

			auto electric = std::stoi(points.substr(0, pos));

			points = points.substr(pos + 1, points.length());

			auto aitAtt = readPoint(points);


			g_raytracer.m_airAtt = aitAtt;









		}
		

	}
  //g_raytracer.m_renderSphapes.push_back(new Sphere(8000, Cen + glm::vec3(0, -8000, -900), glm::vec3(1, .4, .1)*5e-1f));
  /*g_raytracer.m_renderSphapes.push_back(new Sphere(1e4, Cen + glm::vec3(0.631, 0.753, 1.00)*3e-1f, glm::vec3(1, 1, 1)*.5f)); // sky
  g_raytracer.m_renderSphapes.push_back(new Sphere(150,  Cen + glm::vec3(-350,0, -100),  glm::vec3(1,1,1)*.3f  )); // mnt
  g_raytracer.m_renderSphapes.push_back(new Sphere(200,  Cen + glm::vec3(-210,0,-100),   glm::vec3(1,1,1)*.3f  )); // mnt
  g_raytracer.m_renderSphapes.push_back(new Sphere(145,  Cen + glm::vec3(-210,85,-100),  glm::vec3(1,1,1)*.8f  )); // snow
  g_raytracer.m_renderSphapes.push_back(new Sphere(150,  Cen + glm::vec3(-50,0,-100),    glm::vec3(1,1,1)*.3f  )); // mnt
  g_raytracer.m_renderSphapes.push_back(new Sphere(150,  Cen + glm::vec3(100,0,-100),    glm::vec3(1,1,1)*.3f  )); // mnt
  g_raytracer.m_renderSphapes.push_back(new Sphere(125,  Cen + glm::vec3(250,0,-100),    glm::vec3(1,1,1)*.3f  )); // mnt
  g_raytracer.m_renderSphapes.push_back(new Sphere(150,  Cen + glm::vec3(375,0,-100),    glm::vec3(1,1,1)*.3f  )); // mnt
  g_raytracer.m_renderSphapes.push_back(new Sphere(2500, Cen + glm::vec3(0,-2400,-500),  glm::vec3(1,1,1)*.1f  )); // mnt base
  g_raytracer.m_renderSphapes.push_back(new Sphere(8000, Cen + glm::vec3(0,-8000,200),   glm::vec3(.2,.2,1)    )); // water
  g_raytracer.m_renderSphapes.push_back(new Sphere(8000, Cen + glm::vec3(0,-8000,1100),  glm::vec3(0,.3,0)     )); // grass
  g_raytracer.m_renderSphapes.push_back(new Sphere(8   , Cen + glm::vec3(-75, -5, 850),  glm::vec3(0,.3,0)     )); // bush
  g_raytracer.m_renderSphapes.push_back(new Sphere(30,   Cen + glm::vec3(0,   23, 825),  glm::vec3(1,1,1)*.996f)); // ball
  g_raytracer.m_renderSphapes.push_back(new Sphere(30,   Cen + glm::vec3(200,280,-400),    glm::vec3(1,1,1)*.8f));   // clouds
  g_raytracer.m_renderSphapes.push_back(new Sphere(37,   Cen + glm::vec3(237,280,-400),    glm::vec3(1,1,1)*.8f));   // clouds
  g_raytracer.m_renderSphapes.push_back(new Sphere(28,   Cen + glm::vec3(267,280,-400),    glm::vec3(1,1,1)*.8f));   // clouds			   
  g_raytracer.m_renderSphapes.push_back(new Sphere(40,   Cen + glm::vec3(150,280,-1000),    glm::vec3(1,1,1)*.8f ));  // clouds
  g_raytracer.m_renderSphapes.push_back(new Sphere(37,   Cen + glm::vec3(187,280,-1000),    glm::vec3(1,1,1)*.8f ));  // clouds			   
  g_raytracer.m_renderSphapes.push_back(new Sphere(40,   Cen + glm::vec3(600,280,-1100),    glm::vec3(1,1,1)*.8f ));  // clouds
  g_raytracer.m_renderSphapes.push_back(new Sphere(37,   Cen + glm::vec3(637,280,-1100),    glm::vec3(1,1,1)*.8f ));  // clouds			   
  g_raytracer.m_renderSphapes.push_back(new Sphere(37,   Cen + glm::vec3(-800,280,-1400),    glm::vec3(1,1,1)*.8f)); // clouds
  g_raytracer.m_renderSphapes.push_back(new Sphere(37,   Cen + glm::vec3(0,280,-1600),    glm::vec3(1,1,1)*.8f   ));    // clouds
  g_raytracer.m_renderSphapes.push_back(new Sphere(37,   Cen + glm::vec3(537,280,-1800),    glm::vec3(1,1,1)*.8f ));  // clouds*/



	


}

Material Parser::readMaterial(std::string line)
{

	Material actualMat;

	actualMat.diffuseColor = readPoint(line);

	actualMat.emmisionColor = vec3(0.0);

	auto endLimit = line.find_first_of(' ');

	auto pointText = line.substr(endLimit + 1, line.length());

	endLimit = pointText.find_first_of(' ');
	actualMat.specularReflection = std::stof(pointText.substr(0, endLimit));

	pointText = pointText.substr(endLimit + 1, line.length());
	endLimit = pointText.find_first_of(' ');

	actualMat.specularExponent = std::stof(pointText.substr(0, endLimit));

	pointText = pointText.substr(endLimit + 1, pointText.length());

	actualMat.attenuation = readPoint(pointText);

	endLimit = pointText.find_first_of(' ');

	pointText = pointText.substr(endLimit + 1, pointText.length());

	endLimit = pointText.find_first_of(' ');

	actualMat.electricPermittivity = std::stof(pointText.substr(0, endLimit));

	pointText = pointText.substr(endLimit + 1, pointText.length());

	endLimit = pointText.find_first_of(' ');

	actualMat.magneticPermeability = std::stof(pointText.substr(0, endLimit));

	pointText = pointText.substr(endLimit + 1, pointText.length());

	actualMat.roughness = std::stof(pointText);

	return actualMat;
}

glm::vec3 Parser::readPoint(std::string text)
{
	auto initLimit = text.find_first_of('(');
	auto endLimit = text.find_first_of(')');
	size_t last = endLimit - initLimit;
	auto pointText = text.substr(initLimit, last);

	glm::vec3 point;
	auto end_delimit = pointText.find_first_of(',');
	point.x = std::stof(pointText.substr(1, end_delimit - 1));
	auto init_delimit = end_delimit;
	end_delimit = pointText.find_last_of(',');
	auto temp = end_delimit - init_delimit;
	point.y = std::stof(pointText.substr(init_delimit + 1, temp - 1));
	point.z = std::stof(pointText.substr(end_delimit + 1, pointText.length()));

	return point;
}
