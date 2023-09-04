#include "engine/vector.h"
#include "engine/raycast.h"

bool raycast_triangle(float eye[3], float dir[3],
		float **verts, float *distance)
{
	float edge1[3], edge2[3], p[3], t[3], q[3];

	vector_sub(verts[1], verts[0], edge1);
	vector_sub(verts[2], verts[0], edge2);
	vector_cross(dir, edge2, p);

	float det = vector_dot(edge1, p);
	const float epsilon = 0.0001f;
	if(det > -epsilon && det < epsilon)
		return 0;
	
	float inv_det = 1.0f / det;
	vector_sub(eye, verts[0], t);

	float u = inv_det * vector_dot(t, p);

	if(u < 0.0f || u > 1.0f) {
		return 0;
	}
	
	vector_cross(t, edge1, q);

	float v = inv_det * vector_dot(dir, q);

	if (v < 0.0f || u + v > 1.0f) {
		return 0;
	}
	
	float dist = inv_det * vector_dot(edge2, q);

	if(!distance)
		return dist > epsilon;

	*distance = dist;

	return dist > epsilon;
}

