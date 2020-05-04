/* ---------------------------------------------------------------------------------------------------------
Copyright (C) 2020 DigiPen Institute of Technology.
Reproduction or disclosure of this file or its contents without the prior written
consent of DigiPen Institute of Technology is prohibited.
Project: cs500_ivan.iglesias_ 1
Author: Ivan Iglesias, ivan.iglesias, 540000716
Creation date: 1/20/2020
----------------------------------------------------------------------------------------------------------*/
#pragma once
#include "glm/glm.h"
#include <array>
#include <vector>
class Ray;

struct Material
{
	vec3 diffuseColor, attenuation, emmisionColor;
	float specularReflection, specularExponent, electricPermittivity, magneticPermeability, roughness;
};
struct Intersection
{
	Intersection() {};
	Intersection(float time, glm::vec3 point, glm::vec3 normal, Material material)
	{
		m_time = time;
		m_normal = normal;
		m_material = material;
		m_point = point;
	};

	float m_time{-1.0f};
	
	glm::vec3 m_point;
	glm::vec3 m_normal;
	Material m_material;
};

class Shape
{
public:
	virtual Intersection checkIntersection(Ray ray) const = 0;
	Material m_material;

};

class Sphere : public Shape
{
public:

	Sphere() {};
	Sphere(glm::vec3 center, Material mat, float radius);
	Sphere(float radius, glm::vec3 center, glm::vec3 diffuse);

	Intersection checkIntersection(Ray ray) const;

	
private:

	glm::vec3 m_center;
	float m_radius;
};

class Ray
{
public:

	Ray() {};
	Ray(glm::vec3 origin, glm::vec3 direction);

	glm::vec3 getOrigin() const;
	glm::vec3 getDirection() const;

private:

	glm::vec3 m_origin;
	glm::vec3 m_direction;
};

class Plane
{
public:

	Plane() = default;
	Plane(const vec3& normal, const vec3& point);
	Plane(const std::vector<vec3> vertices);


	vec3 getPoint() const { return m_point; };
	vec3 getNormal() const { return m_normal; };

	void set(vec3 point, vec3 normal);

	vec2 projectPoint(vec3 vert) const;

	Intersection checkIntersection(Ray ray) const;

private: 
	glm::vec3  m_normal, m_point;
	int m_dominantAxis;

};

class Box : public Shape
{
public:
	// In this order: left, right, top, bottom, near, far


	Box(glm::vec3 boxCorner, glm::vec3 length, glm::vec3 width, glm::vec3 height, Material mat);

	Intersection checkIntersection(Ray ray) const;

	const Plane& get_left() const { return m_planes[0]; }
	const Plane& get_right() const { return m_planes[1]; }
	const Plane& get_top() const { return m_planes[2]; }
	const Plane& get_bottom() const { return m_planes[3]; }
	const Plane& get_near() const { return m_planes[4]; }
	const Plane& get_far() const { return m_planes[5]; }


private: 
	glm::vec3 m_length, m_width, m_height;
	std::array<Plane, 6> m_planes;

};

class Polyg : public Shape
{
public:
	Polyg() {};
	Polyg(int vertNum, std::vector<glm::vec3> vertex, Material mat);

	Intersection checkIntersection(Ray ray) const;


private:
	int m_vertNum;
	std::vector<glm::vec3> m_vertex;
	unsigned m_dominantAxis;
	Plane polygPlane;
	

};

class Ellipsoid : public Shape
{
public:
	Ellipsoid() {};
	Ellipsoid(vec3 center, vec3 uVec, vec3 vVec, vec3 wVec, Material mat);

	Intersection checkIntersection(Ray ray) const;

private:
	glm::vec3 center, uVec, vVec, wVec;
	glm::mat3 matrixTransform, matrixTransformInv;


};


class Light : public Shape
{
public:

	Light() {};
	Light(glm::vec3 center, glm::vec3 diffuse, float radius, int id);

	Intersection checkIntersection(Ray ray) const;

	glm::vec3 getColor() const;
	glm::vec3 getPos() const;
	float getRadius() const;
	
	int light_id = 0;
private:
	glm::vec3 m_position;
	glm::vec3 m_color;
	glm::vec3 m_emission;
	float m_radius;
};
