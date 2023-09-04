#include "engine/util.h"

float clampf(float x, float min, float max)
{
	if(x > max)
		return max;
	if(x < min)
		return min;
	return x;
}

long unsigned int uint32_flip_endian(long unsigned int v)
{
	return ((v >> 24) & 0xff) | ((v << 8) & 0xff0000) | 
		((v >> 8) & 0xff00) | ((v << 24) & 0xff000000);
}

unsigned int uint16_flip_endian(unsigned int v)
{
	return (v >> 8) | (v << 8);
}

float lerpf(float a, float b, float t)
{
	return a + (b - a) * clampf(t, 0, 1);
}
