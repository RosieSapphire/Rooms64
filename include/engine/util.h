#ifndef ENGINE_UTIL_H_
#define ENGINE_UTIL_H_

#define PI 3.14159265358979323846f
#define PI_HALF 1.57079632679489661923f

#define TO_RADIANS (PI / 180.0f)
#define TO_DEGREES (180.0f / PI)

float clampf(float x, float min, float max);
long unsigned int uint32_flip_endian(long unsigned int v);
unsigned int uint16_flip_endian(unsigned int v);
float lerpf(float a, float b, float t);

#endif /* ENGINE_UTIL_H_ */
