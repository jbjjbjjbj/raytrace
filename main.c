#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "vector.h"
#include "viewpoint.h"
#include "ray.h"

typedef struct {
	viewpoint_t view;
} space_t;

int main()
{
	printf("Starting\n");

	space_t s;

	vector_set(&s.view.position, 10, 20, 10);
	vector_set(&s.view.target, 0, 0, 0);
	s.view.width = 100;
	s.view.height = 100;

	viewpoint_init(&s.view);

	ray_t r;
	r.start = &s.view.position;
	viewpoint_ray(&s.view, &r.direction, 66, 33);

	vector_print(&r.direction);

	plane_t pl;
	pl.start = vector_set(NULL, 0, 0, 12);
	vector_set(&pl.norm, 6, 2, 0);
	vector_scale_inv(&pl.norm, &pl.norm, vector_len(&pl.norm));
	vector_print(&pl.norm);

	printf("Intersect distance %f\n", ray_intersect_plane(&pl, &r));

	
}
