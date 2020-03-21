#ifndef RAY_H
#define RAY_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "scene.h"

// Random COORD_T between 0 and 1
extern COORD_T ray_rand(void *seed);

typedef struct {
    // Start is not unique so it's a pointer to save copying time
    vector_t *start;
    vector_t *direction;
} ray_t;

COORD_T ray_intersect(object_t *o, ray_t *ray, bool skip_dist);
COORD_T ray_intersect_sphere(sphere_t *s, ray_t *ray, bool skip_dist);
COORD_T ray_intersect_plane(plane_t *p, ray_t *ray, bool skip_dist);

object_t *ray_cast(space_t *s, ray_t *r, COORD_T *dist_ret, bool chk, COORD_T chk_dist);
void ray_trace(space_t *s, unsigned int x, unsigned int y, unsigned samples, color_t *c, void *seed);

#endif
