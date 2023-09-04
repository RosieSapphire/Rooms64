#ifndef ENGINE_VECTOR_H_
#define ENGINE_VECTOR_H_

#include <GL/gl.h>

void vector_copy(GLfloat a[3], GLfloat b[3]);
void vector_add(GLfloat a[3], GLfloat b[3], GLfloat o[3]);
void vector_sub(GLfloat a[3], GLfloat b[3], GLfloat o[3]);
void vector_scale_to(GLfloat x[3], GLfloat s, GLfloat o[3]);
void vector_scale(GLfloat x[3], GLfloat s);
GLfloat vector_dot(GLfloat a[3], GLfloat b[3]);
void vector_cross(GLfloat a[3], GLfloat b[3], GLfloat o[3]);
float vector_magnitude(GLfloat v[3]);
float vector_normalize(GLfloat v[3]);
float vector_distance(GLfloat a[3], GLfloat b[3]);
void vector_lerp(GLfloat a[3], GLfloat b[3], GLfloat t, GLfloat o[3]);
void vector_invert(GLfloat i[3], GLfloat o[3]);

#endif /* ENGINE_VECTOR_H_ */
