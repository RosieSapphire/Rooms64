#ifndef ENGINE_RAYCAST_H_
#define ENGINE_RAYCAST_H_

#include <stdbool.h>

bool raycast_triangle(float eye[3], float dir[3],
		float **verts, float *distance);

#endif /* ENGINE_RAYCAST_H_ */
