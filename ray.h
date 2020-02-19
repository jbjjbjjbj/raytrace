#ifndef RAY_H
#define RAY_H

#include "vector.h"

typedef struct {
    // Start is not unique so it's a pointer to save copying time
    vector_t *start;

    vector_t direction;
} ray_t;

typedef struct {
	vector_t *center;
	COORD_T radius;
} sphere_t;

// TODO make this less inconsistent
typedef struct {
	vector_t *start;
	vector_t norm;
} plane_t;


COORD_T ray_intersect_sphere(sphere_t *s, ray_t *ray);
COORD_T ray_intersect_plane(plane_t *p, ray_t *ray);

#endif
