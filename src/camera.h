/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_ivan.iglesias_ 1
Author: Ivan Iglesias, ivan.iglesias, 540000716
Creation date: 1/20/2020
----------------------------------------------------------------------------------------------------------*/
#pragma once
#include "glm/glm.hpp"
class Camera
{
public:
	Camera() {};
	~Camera() {};

	glm::vec3 m_position;
	glm::vec3 m_up;
	glm::vec3 m_right;
	float m_fov;
private:

};

