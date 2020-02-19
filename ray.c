#include <math.h>
#include "vector.h"

#include "ray.h"

// https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
// http://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection/
COORD_T ray_intersect_sphere(sphere_t *s, ray_t *ray)
{
    // Vector between vector start and center of circle
    vector_t oc;
    vector_sub(&oc, ray->start, s->center);

    // Solve quadratic function
	// TODO Not sure if this step i neccesary because dir is unit
    COORD_T a = vector_dot(&ray->direction, &ray->direction);
    COORD_T b = 2 * vector_dot(&oc, &ray->direction);
    COORD_T c = vector_dot(&oc, &oc) - s->radius * s->radius;

    COORD_T d = b * b - 4 * a * c;

    // no intersection
    if (d < 0) {
        return -1;
    }

    // Else take the closest intersection
    return (-b - sqrt(d) ) / (2.0 * a);
}

// Requires that vectors are normalized
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection
COORD_T ray_intersect_plane(plane_t *p, ray_t *ray)
{
	// If zero ray is parralel to plane
	COORD_T nr = vector_dot(&p->norm, &ray->direction);
	vector_print(&ray->direction);
	//
	// Take care of rounding errors
	if (nr < ZERO_APROX && nr > -ZERO_APROX) {
		return -1;
	}

	// Calculate distance
	vector_t tmp;
	vector_copy(&tmp, p->start);
	vector_sub(&tmp, &tmp, ray->start);

	COORD_T t = vector_dot(&tmp, &p->norm) / nr;
	return t;
}
