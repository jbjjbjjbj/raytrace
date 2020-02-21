#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "vector.h"
#include "ray.h"
#include "scene.h"
#include "pgm.h"

#define TESTW 1000
#define TESTH 1000

color_t back = {0, 0, 0};

int main()
{
	//printf("Starting\n");
	space_t s;
	s.objects = NULL;
	s.lights = NULL;

	material_t m;
	vector_set(&m.color, 0, 1, 0);

	vector_set(&s.view.position, 0, 15, 10);
	vector_set(&s.view.target, 0, 0, 0);
	s.view.width = TESTW;
	s.view.height = TESTH;

	viewpoint_init(&s.view);
	
	// Setup plane
	add_sphere(&s, vector_set(NULL, 0, 0, 5), 5, &m);
	//add_sphere(&s, vector_set(NULL, 10, 0, 5), 5);
	add_plane(&s, vector_set(NULL, 0, 0, 2), vector_set(NULL, 0, 0, 1));
	add_light(&s, vector_set(NULL, 10, 0, 20), color_set(NULL, 255, 255, 255));

	pgm_write_header(stdout, TESTW, TESTH);
	for (int y = TESTH; y; y--) {
		for (int x = TESTW; x; x--) {
			color_t *c = ray_trace(&s, x, y);

			if (c) {
				pgm_write_pixel(stdout, c);
			} else {
				pgm_write_pixel(stdout, &back);
			}
			free(c);
		}
	}
	
}
