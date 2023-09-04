#include <math.h>
#include <string.h>

#include "engine/util.h"
#include "engine/vector.h"

void vector_copy(GLfloat a[3], GLfloat b[3])
{
	memcpy(b, a, sizeof(float[3]));
}

void vector_add(GLfloat a[3], GLfloat b[3], GLfloat o[3])
{
	for(int i = 0; i < 3; i++)
		o[i] = a[i] + b[i];
}

void vector_sub(GLfloat a[3], GLfloat b[3], GLfloat o[3])
{
	for(int i = 0; i < 3; i++)
		o[i] = a[i] - b[i];
}

void vector_scale_to(GLfloat x[3], GLfloat s, GLfloat o[3])
{
	for(int i = 0; i < 3; i++)
		o[i] = x[i] * s;
}

void vector_scale(GLfloat x[3], GLfloat s)
{
	vector_scale_to(x, s, x);
}

GLfloat vector_dot(GLfloat a[3], GLfloat b[3])
{
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

void vector_cross(GLfloat a[3], GLfloat b[3], GLfloat o[3])
{
	o[0] = a[1] * b[2] - a[2] * b[1];
	o[1] = a[2] * b[0] - a[0] * b[2];
	o[2] = a[0] * b[1] - a[1] * b[0];
}

float vector_magnitude(GLfloat v[3])
{
	float mag = sqrtf(vector_dot(v, v));
	if(mag < 0.0001f)
		return 0.0f;
	return mag;
}

float vector_normalize(GLfloat v[3])
{
	float mag = vector_magnitude(v);
	if(!mag)
		return mag;

	vector_scale(v, 1.0f / mag);
	return mag;
}

float vector_distance(GLfloat a[3], GLfloat b[3])
{
	float o[3];
	vector_sub(b, a, o);
	return vector_magnitude(o);
}

void vector_lerp(GLfloat a[3], GLfloat b[3], GLfloat t, GLfloat o[3])
{
	for(int i = 0; i < 3; i++)
		o[i] = lerpf(a[i], b[i], t);
}

void vector_invert(GLfloat i[3], GLfloat o[3])
{
	for(int j = 0; j < 3; j++)
		o[j] = -i[j];
}
