#include "MLT.h"

MLT::MLT()
{
	global_time = large_step = large_step_time = used_rand_coords = 0;
	for (int i = 0; i < 128; i++)
		primary_samples.push_back(PrimarySample(xor128_));
}

float MLT::Mutate(const float  x)
{
	const float r = static_cast<float>(xor128_.rand01());
	const float s1 = 1.0f / 512.0f;
	const float	s2 = 1.0f / 16.0f;
	const float dx = s1 / (s1 / s2 + fabsf(2.0f * r - 1.0f)) - s1 / (s1 / s2 + 1.0f);
	if (r < 0.5f)
	{
		const float x1 = x + dx;
		return (x1 < 1.0f) ? x1 : x1 - 1.0f;
	}
	else {
		const float x1 = x - dx;
		return (x1 < 0.0f) ? x1 + 1.f : x1;
	}
}

float MLT::NextSample()
{
	if (primary_samples.size() <= used_rand_coords)
	{
		for (int i = 0; i < 128; i++)
			primary_samples.push_back(PrimarySample(xor128_));
	}

	if (primary_samples[used_rand_coords].modifyTime < global_time)
	{
		//Compute Large Step
		if (large_step > 0)
		{
			primary_samples_stack.push(primary_samples[used_rand_coords]);
			primary_samples[used_rand_coords].modifyTime = global_time;
			primary_samples[used_rand_coords].value = xor128_.rand01();
		}
		else //Compute small step
		{
			if (primary_samples[used_rand_coords].modifyTime < large_step_time)
			{
				primary_samples[used_rand_coords].modifyTime = large_step_time;
				primary_samples[used_rand_coords].value = xor128_.rand01();
			}

			//Mutate the stored values
			while (primary_samples[used_rand_coords].modifyTime < global_time - 1)
			{
				primary_samples[used_rand_coords].value = Mutate(primary_samples[used_rand_coords].value);
				primary_samples[used_rand_coords].modifyTime++;
			}
			primary_samples_stack.push(primary_samples[used_rand_coords]);
			primary_samples[used_rand_coords].value = Mutate(primary_samples[used_rand_coords].value);
			primary_samples[used_rand_coords].modifyTime = global_time;
		}
	}
	//Update the position in the array
	used_rand_coords++;
	return primary_samples[used_rand_coords - 1].value;
}

//Calculate the luminance of a Color
float luminance(const glm::vec3 &color)
{
	return glm::dot(glm::vec3(0.2126, 0.7152, 0.0722), glm::vec3(color));
}
