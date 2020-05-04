/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_ivan.iglesias_ 1
Author: Ivan Iglesias, ivan.iglesias, 540000716
Creation date: 1/20/2020
----------------------------------------------------------------------------------------------------------*/
#pragma once
#include <string>
#include <glm/glm.hpp>
#include "model.h"

class Parser
{
public:

	Parser() {};
	~Parser() {};

	static void readFileObjects(std::string filename);

private:

	static Material readMaterial(std::string line);
	static glm::vec3 readPoint(std::string pointText);
};

