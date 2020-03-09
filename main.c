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
	color_set(&s.back, 0, 0, 0);

	material_t m;
	vector_set(&m.color, 1, 1, 1);
	m.defuse = 0.0;
	m.specular = 0.0;
	m.shine = 80;
	m.reflective = 0.5;

	material_t m2;
	vector_set(&m2.color, 0.1, 1, 0.7);
	m2.defuse = 0.9;
	m2.specular = 1;
	m2.shine = 60;
	m2.reflective = 0.1;

	material_t mpl;
	//vector_set(&mpl.color, 0, 0.396, 0.7019);
	vector_set(&mpl.color, 1, 1, 1);
	mpl.defuse = 1;
	mpl.specular = 0.0;
	mpl.shine = 50;
	mpl.reflective = 0.0;

	vector_set(&s.view.position, 0, 14, 5);
	vector_set(&s.view.target, 0, 0, 5);
	s.view.width = TESTW;
	s.view.height = TESTH;

	viewpoint_init(&s.view);
	
	// Setup plane
	add_sphere(&s, vector_set(NULL, 0, 4, 5), 5, &m2);
	add_sphere(&s, vector_set(NULL, 8, 8, 3), 2, &m2);
	add_plane(&s, vector_set(NULL, 0, 0, 2), vector_set(NULL, 0, 0, 1), &mpl);
	//add_plane(&s, vector_set(NULL, 0, -20, 0), vector_set(NULL, 0, 1, 0), &mpl);
	add_light(&s, vector_set(NULL, 20, 10, 30), color_set(NULL, 1, 1, 1), color_set(NULL, 0.5, 0.5, 0.5));
	//add_light(&s, vector_set(NULL, 0, 10, 20), color_set(NULL, 255, 255, 255), color_set(NULL, 150, 150, 150));

	pgm_write_header(stdout, TESTW, TESTH);
	for (int y = TESTH; y; y--) {
		for (int x = TESTW; x; x--) {
			color_t *c = ray_trace(&s, x, y);

			if (c) {
				pgm_write_pixel(stdout, c);
			} 
			free(c);
		}
	}
	
}
