/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_ivan.iglesias_ 1
Author: Ivan Iglesias, ivan.iglesias, 540000716
Creation date: 1/20/2020
----------------------------------------------------------------------------------------------------------*/
#include "model.h"
#include <iostream>


Sphere::Sphere(glm::vec3 center, Material mat, float radius)
{
	m_center = center;
	m_material = mat;
	m_radius = radius;
}

Sphere::Sphere(float radius, glm::vec3 center, glm::vec3 diffuse)
{
	m_center = center;
	m_radius = radius;

	Material actual;

	actual.diffuseColor = diffuse;
	actual.specularReflection = 0.0f;
	actual.specularExponent = 0.1f;



	m_material = actual;
}

Intersection Sphere::checkIntersection(Ray ray) const
{	
	auto vec = ray.getOrigin() - m_center;

	if (ray.getDirection() == glm::zero<vec3>())
	{
		return Intersection();
	}
	float length = glm::length(ray.getDirection());
	float a = length * length;
	float b = 2.0f * glm::dot(ray.getDirection(), vec);

	float c = glm::dot(vec, vec) - m_radius * m_radius;
	float discr = b * b - 4.0f * a* c;
	//Exit if origin is outside and not pointing
	float t;
	if (discr < 0.0f)
		return Intersection();
	else
	{
		float t1 = (-b + glm::sqrt(discr)) / (2.0f * a);
		float t2 = (-b - glm::sqrt(discr)) / (2.0f * a);

		if (t1 < 0.0f)
			return Intersection();
		else if (t2 < 0.0f)
			t = t1;
		else
			t = t2;

	}	

	//compute intersection point and normal
	glm::vec3 interPoint = ray.getOrigin() + ray.getDirection() * t;
	glm::vec3 interNormal = glm::normalize(interPoint - m_center);

	return Intersection(t, interPoint, interNormal, m_material);
}



Ray::Ray(glm::vec3 origin, glm::vec3 direction)
{
	m_origin = origin;
	m_direction = direction;
}

glm::vec3 Ray::getOrigin() const
{
	return m_origin;
}

glm::vec3 Ray::getDirection()const
{
	return m_direction;
}

Plane::Plane(const vec3 & normal, const vec3 & point)
{
	m_normal = normal;
	m_point = point;

}

Plane::Plane(const std::vector<vec3> vertices)
{
	vec3 vec1 = vertices[1] - vertices[0];
	vec3 vec2 = vertices[2] - vertices[1];

	auto normal = glm::normalize(glm::cross(vec1, vec2));

	auto absNormal = glm::abs(normal);

	if (absNormal.x > absNormal.y)
	{
		if (absNormal.x > absNormal.z)
			m_dominantAxis = 0;
		else
			m_dominantAxis = 2;
	}
	else
	{
		if (absNormal.y > absNormal.z)
			m_dominantAxis = 1;
		else
			m_dominantAxis = 2;
	}

	m_normal = normal;
	m_point = vertices[0];

}

void Plane::set(vec3 point, vec3 normal)
{
	m_point = point;
	m_normal = normal;
}

vec2 Plane::projectPoint(vec3 vert) const
{
	vec2 result;
	switch (m_dominantAxis)
	{
	case 0:
		result.x = vert.y;
		result.y = vert.z;
		break;
	case 1:
		result.x = vert.z;
		result.y = vert.x;
		break;
	case 2:
		result.x = vert.x;
		result.y = vert.y;
		break;
	default:
		result.x = vert.x;
		result.y = vert.y;
		break;
	}
	
	return result;
}

Intersection Plane::checkIntersection(Ray ray) const
{
	//check with actual plane normal and point
	vec3 planeNormal = getNormal();
	vec3 planePoint = getPoint();

	float b = glm::dot(ray.getDirection(), planeNormal);

	if(std::abs(b) == 0.0)
		return Intersection();

	vec3 dir = planePoint - ray.getOrigin();

	float time = glm::dot(dir, planeNormal) / b;

	if(time < 0.0)
		return Intersection();
	else
	{
		glm::vec3 interPoint = ray.getOrigin() + ray.getDirection() * time;
		return Intersection(time, interPoint, planeNormal, Material());
	}
		
}


Box::Box(glm::vec3 boxCorner, glm::vec3 length, glm::vec3 width, glm::vec3 height, Material mat)
{

	m_height = height;
	m_width = width;
	m_length = length;

	m_material = mat;

	//create the planes with the box information

	vec3 actualPoint, actualNormal;
	//front 
	actualPoint = boxCorner;
	actualNormal = glm::cross(length, height);
	m_planes[0].set(actualPoint, actualNormal);

	//back
	actualPoint = boxCorner + width;
	actualNormal = glm::cross(height, length);
	m_planes[1].set(actualPoint, actualNormal);

	//left
	actualPoint = boxCorner;
	actualNormal = glm::cross(height, width);
	m_planes[2].set(actualPoint, actualNormal);

	//right
	actualPoint = boxCorner + length;
	actualNormal = glm::cross(width, height);
	m_planes[3].set(actualPoint, actualNormal);

	//bottom
	actualPoint = boxCorner;
	actualNormal = glm::cross(width, length);
	m_planes[4].set(actualPoint, actualNormal);

	//bottom
	actualPoint = boxCorner + height;
	actualNormal = glm::cross(length, width);
	m_planes[5].set(actualPoint, actualNormal);


}

Intersection Box::checkIntersection(Ray ray) const
{
	float tMin = 0.0f;
	float tMax = FLT_MAX;
	float tTemp = 0.0f;

	glm::vec3 normalMin(0.0f);
	glm::vec3 normalMax(0.0f);

	int i = 0;

	vec3 rayDir = ray.getDirection();
	vec3 rayOrigin = ray.getOrigin();

	//iterate the planes
	while (i <= 5 && tMin <= tMax)
	{
		//check with actual plane normal and point
		vec3 planeNormal = m_planes[i].getNormal();
		vec3 planePoint = m_planes[i].getPoint();

		float b = glm::dot(rayDir, planeNormal);

		if (b < 0.0f)
		{
			//compute time and compare with the previous stores value
			tTemp = -glm::dot(rayOrigin - planePoint, planeNormal) / b;

			if (tTemp == std::max(tMin, tTemp))
			{
				tMin = tTemp;
				normalMin = planeNormal;
			}
		}
		else if (b > 0.0f)
		{
			tTemp = -glm::dot(rayOrigin - planePoint, planeNormal) / b;

			if (tTemp == std::min(tMax, tTemp))
			{
				tMax = tTemp;
				normalMax = planeNormal;
			}
			
		}
		else if (glm::dot((rayOrigin - planePoint), planeNormal) > 0.0f)
		{
			tMin = tMax + 1.0f;

		}
		i++;

	}

	//depending on the values of t min and max chose different normal
	float finalTime;
	glm::vec3 finalNormal;
	if (tMin >= tMax)
		return Intersection();
	else if (tMin <= 0)
	{
		finalTime = tMax;
		finalNormal = normalMax;
	}
	else
	{
		finalTime = tMin;
		finalNormal = normalMin;
	}

	//Compute point of intersection

	glm::vec3 interPoint = rayOrigin + rayDir * finalTime;

	return Intersection(finalTime, interPoint, glm::normalize(finalNormal), m_material);

}


Light::Light(glm::vec3 center, glm::vec3 diffuse, float radius, int id)
{
	m_position = center;
	m_color = diffuse;
	m_radius = radius;
	light_id = id;
}

Intersection Light::checkIntersection(Ray ray) const
{
	auto vec = ray.getOrigin() - m_position;

	if (ray.getDirection() == glm::zero<vec3>())
	{
		return Intersection();
	}
	float length = glm::length(ray.getDirection());
	float a = length * length;
	float b = 2.0f * glm::dot(ray.getDirection(), vec);

	float c = glm::dot(vec, vec) - m_radius * m_radius;
	float discr = b * b - 4.0f * a* c;
	//Exit if origin is outside and not pointing
	float t;
	if (discr < 0.0f)
		return Intersection();
	else
	{
		float t1 = (-b + glm::sqrt(discr)) / (2.0f * a);
		float t2 = (-b - glm::sqrt(discr)) / (2.0f * a);

		if (t1 < 0.0f)
			return Intersection();
		else if (t2 < 0.0f)
			t = t1;
		else
			t = t2;
	}

	//compute intersection point and normal
	glm::vec3 interPoint = ray.getOrigin() + ray.getDirection() * t;
	glm::vec3 interNormal = glm::normalize(interPoint - m_position);

	return Intersection(t, interPoint, interNormal, m_material);
}

glm::vec3 Light::getColor() const
{
	return m_color;
}

glm::vec3 Light::getPos() const 
{
	return m_position;
}

float Light::getRadius() const
{
	return m_radius;
}

Polyg::Polyg(int vertNum, std::vector<glm::vec3> vertex, Material mat)
{
	this->m_vertNum = vertNum;
	this->m_vertex = vertex;
	this->m_material = mat;

	Plane actualPlane(vertex);

	polygPlane = actualPlane;

}

Intersection Polyg::checkIntersection(Ray ray) const
{

	Intersection intersect = polygPlane.checkIntersection(ray);

	if (intersect.m_time < 0.0)
		return Intersection();

	vec2 interPointProj = polygPlane.projectPoint(intersect.m_point);

	std::vector<vec2> projectPoints;
	for (unsigned i = 0; i < m_vertex.size(); i++)
	{
		projectPoints.push_back(polygPlane.projectPoint(m_vertex[i]) - interPointProj);
	}
	projectPoints.push_back(projectPoints.front());
	int n = 0;

	for (unsigned i = 0; i < projectPoints.size() - 1; i++)
	{
		vec2 start = projectPoints[i];
		vec2 end = projectPoints[i + 1];

		if(start.x >= end.x)
		{
			std::swap(start, end);
		}
		if (start.x < 0 && end.x >= 0 && (start.y > 0 || end.y >= 0))
		{
			float dx = end.x - start.x;
			float dy = end.y - start.y;

			float m = dy / dx;
			float b = start.y - m * start.x;

			if (b >= 0)
				n++;
		}
	}
	if ((n % 2) == 1)
	{
		intersect.m_material = m_material;
		return intersect;
	}

	return Intersection();
}

Ellipsoid::Ellipsoid(vec3 center, vec3 uVec, vec3 vVec, vec3 wVec, Material mat)
{
	matrixTransform = glm::mat3(uVec, vVec, wVec);
	matrixTransformInv = glm::inverse(matrixTransform);

	this->uVec = uVec;
	this->vVec = vVec;
	this->wVec = wVec;
	this->center = center;
	m_material = mat;


}

Intersection Ellipsoid::checkIntersection(Ray ray) const
{

	glm::vec3 dirPrime = matrixTransformInv * ray.getDirection();

	glm::vec3 p0Prime = matrixTransformInv * (ray.getOrigin() - center);


	if (ray.getDirection() == glm::zero<vec3>())
	{
		return Intersection();
	}

	float length = glm::length(dirPrime);
	float a = length * length;
	float b = 2.0f * glm::dot(dirPrime, p0Prime);

	float c = glm::dot(p0Prime, p0Prime) - 1.0f;
	float discr = b * b - 4.0f * a * c;
	//Exit if origin is outside and not pointing
	float t;
	if (discr < 0.0f)
		return Intersection();
	else
	{
		float t1 = (-b + glm::sqrt(discr)) / (2.0f * a);
		float t2 = (-b - glm::sqrt(discr)) / (2.0f * a);

		if (t1 < 0.0f)
			return Intersection();
		else if (t2 < 0.0f)
			t = t1;
		else
			t = t2;
	}

	//compute intersection point and normal
	glm::vec3 interPoint = ray.getOrigin() + ray.getDirection() * t;

	glm::vec3 interNormal = glm::normalize(glm::transpose(matrixTransformInv) * matrixTransformInv * (interPoint - center));

	return Intersection(t, interPoint, interNormal, m_material);
	
}
