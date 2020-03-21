#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "vector.h"
#include "ray.h"
#include "scene.h"
#include "pgm.h"

#define TESTW 1000
#define TESTH 1000

int main()
{
	//printf("Starting\n");
	space_t s;
	s.objects = NULL;
	s.lights = NULL;
	color_set(&s.ambient, 0.09, 0.09, 0.09);
	// Currently havin issues with white background
	// color_set(&s.back, 255, 255, 255);
	color_set(&s.back, 0.8, 0.8, 0.8);
	color_set(&s.env_color, 0.13, 0.13, 0.13);
	s.env_samples = 256;

	material_t m;
	vector_set(&m.color, 0.4, 0.4, 1);
	m.defuse = 1;
	m.specular = 0.0;
	m.shine = 80;
	m.reflective = 0.0;

	material_t m3;
	vector_set(&m3.color, 1, 0.3, 0.1);
	m3.defuse = 0.95;
	m3.specular = 0.0;
	m3.shine = 80;
	m3.reflective = 0.05;

	material_t m2;
	vector_set(&m2.color, 1, 1, 1);
	m2.defuse = 0.0;
	m2.specular = 0.5;
	m2.shine = 80;
	m2.reflective = 1;

	material_t mpl;
	//vector_set(&mpl.color, 0, 0.396, 0.7019);
	vector_set(&mpl.color, 1, 1, 1);
	mpl.defuse = 1;
	mpl.specular = 0.0;
	mpl.shine = 50;
	mpl.reflective = 0.0;

	vector_set(&s.view.position, 0, 16, 6);
	vector_set(&s.view.target, 0, 0, 6);
	s.view.width = TESTW;
	s.view.height = TESTH;

	viewpoint_init(&s.view);
	
	// Setup plane
	add_sphere(&s, vector_set(NULL, 0, 4, 7), 5, &m2);
	add_sphere(&s, vector_set(NULL, 8, 8, 4), 2, &m3);
	add_sphere(&s, vector_set(NULL, -10, 9, 5), 3, &m);
	add_sphere(&s, vector_set(NULL, -10, -5, 5), 3, &m);
	add_plane(&s, vector_set(NULL, 0, 0, 2), vector_set(NULL, 0, 0, 1), &mpl);
	//add_plane(&s, vector_set(NULL, 0, -20, 0), vector_set(NULL, 0, 1, 0), &mpl);
	add_light(&s, vector_set(NULL, 20, 10, 30), color_set(NULL, 0.3, 0.3, 0.3), color_set(NULL, 0.5, 0.5, 0.5));
	//add_light(&s, vector_set(NULL, 0, 10, 20), color_set(NULL, 0.5, 0.5, 0.5), color_set(NULL, 0.5, 0.5, 0.5));

	pgm_write_header(stdout, TESTW, TESTH);

	// Height percentage
	unsigned percentstep = TESTH / 100;
	unsigned percent = 0;

	for (int y = TESTH; y; y--) {
		for (int x = TESTW; x; x--) {
			color_t *c = ray_trace(&s, x, y, 2);

			if (c) {
				pgm_write_pixel(stdout, c);
			} 
			free(c);

		}

		if (y % percentstep == 0) {
			fprintf(stderr, "%d%\n", percent++);
		}
	}
	
}
