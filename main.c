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
int print = 0;

int main()
{
	//printf("Starting\n");
	space_t s;
	s.objects = NULL;
	s.lights = NULL;
	color_set(&s.ambient, 30, 30, 30);

	material_t m;
	vector_set(&m.color, 0, 1, 0);
	m.defuse = 1;
	m.specular = 0.7;
	m.shine = 20;

	material_t mpl;
	vector_set(&mpl.color, 0, 0.396, 0.7019);
	mpl.defuse = 1;
	mpl.specular = 0.3;
	mpl.shine = 20;

	vector_set(&s.view.position, 0, 15, 8);
	vector_set(&s.view.target, 0, 0, 0);
	s.view.width = TESTW;
	s.view.height = TESTH;

	viewpoint_init(&s.view);
	
	// Setup plane
	add_sphere(&s, vector_set(NULL, 3, 0, 5), 5, &m);
	add_sphere(&s, vector_set(NULL, -6, -3, 7), 3, &m);
	//add_sphere(&s, vector_set(NULL, 10, 0, 5), 5);
	add_plane(&s, vector_set(NULL, 0, 0, 2), vector_set(NULL, 0, 0, 1), &mpl);
	add_light(&s, vector_set(NULL, 10, 0, 20), color_set(NULL, 150, 150, 150), color_set(NULL, 150, 150, 150));

	pgm_write_header(stdout, TESTW, TESTH);
	for (int y = TESTH; y; y--) {
		for (int x = TESTW; x; x--) {
			if (0 && y <= 500) {
				printf("Lets go");
				print = 1;
			}
			color_t *c = ray_trace(&s, x, y);

			if (c) {
				pgm_write_pixel(stdout, c);
			} else {
				pgm_write_pixel(stdout, &back);
			}
			free(c);
			print = 0;
		}
	}
	
}
