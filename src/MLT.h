#pragma once
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <string>
#include <vector>
#include <stack>
#include "glm/glm.h"
#include <mutex>
#include <random>

//#include "raytracer.h"


namespace RANDOM
{

	
	class xor128
	{
	public:
		unsigned int x, y, z, w;
		xor128()
		{
			x = 123456789;
			y = 362436069;
			z = 521288629;
			w = 88675123;
		}
		inline unsigned int step(void)
		{
			unsigned int t;
			t = x ^ (x << 11);
			x = y; y = z; z = w;
			return w = (w ^ (w >> 19)) ^ (t ^ (t >> 8));
		}
		void setSeed(unsigned u)
		{
			x ^= u;
		}
		inline float rand01() { return static_cast<float>((double)step() / (UINT_MAX)); }
	};

}



//Structure use to store the information of each sample made up by random numbers
struct PrimarySample 
{
	int modifyTime;
	float value;
	PrimarySample(RANDOM::xor128& valxor)
	{
		modifyTime = 0;
		value = valxor.rand01();
	}
};


//Store the information of the paths
struct PathSample
{
	int x;
	int y;
	glm::vec3 Color;
	float weight;

	PathSample(const int x_ = 0, const int y_ = 0, const glm::vec3 &color_ = glm::vec3(0.0f), const float weight_ = 1.0f) :
		x(x_), y(y_), Color(color_), weight(weight_) {}
};



//This class stores all the information about the 
class MLT
{

public:
	RANDOM::xor128 xor128_;
	int global_time;
	int large_step;
	int large_step_time;
	int used_rand_coords;

	std::vector<PrimarySample> primary_samples;
	std::stack<PrimarySample> primary_samples_stack;

	MLT();

	void ResetRandomCoords() {	used_rand_coords = 0;	}

	inline double rand01(void) { return xor128_.rand01(); }
	float NextSample();

private:

	float Mutate(const float  x);
	
};

float luminance(const glm::vec3 &color);






