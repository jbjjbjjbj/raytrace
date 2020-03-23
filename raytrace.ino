#include <stdio.h>
#include <math.h>
#include <stdlib.h>

#include "vector.h"
#include "ray.h"
#include "scene.h"
#include "pgm.h"

#define TESTW 320
#define TESTH 240

char container[ CONTAINER_SIZE(5, 4, 1) ];

// Implement random 
COORD_T ray_rand(void *seed)
{
	return (COORD_T) random(10000) / 10000;
}

void setup()
{
	Serial.begin(115200);
	Serial.println("Starting");

	container_t *cont = (container_t *) container;
	container_init(cont, 5, 4, 1);
	
	// Init space_t
	space_t *s = container_prepare_space(cont);
	Serial.println("Starting");
	
	// Set space options
	color_set(&s->ambient, 0.09, 0.09, 0.09);
	color_set(&s->back, 0.8, 0.8, 0.8);
	color_set(&s->env_color, 0.13, 0.13, 0.13);
	s->env_samples = 256;

	// Set viewpoint options
	vector_set(&s->view.position, 0, 16, 6);
	vector_set(&s->view.target, 0, 0, 6);
	s->view.width = TESTW;
	s->view.height = TESTH;
	Serial.println("Starting");

	// Create materials
	material_t *m = add_material(cont);
	vector_set(&m->color, 0.4, 0.4, 1);
	m->defuse = 1;
	m->specular = 0.0;
	m->shine = 80;
	m->reflective = 0.0;

	material_t *m3 = add_material(cont);
	vector_set(&m3->color, 1, 0.3, 0.1);
	m3->defuse = 0.95;
	m3->specular = 0.0;
	m3->shine = 80;
	m3->reflective = 0.05;
	Serial.println("Starting");

	material_t *m2 = add_material(cont);
	vector_set(&m2->color, 1, 1, 1);
	m2->defuse = 0.0;
	m2->specular = 0.5;
	m2->shine = 80;
	m2->reflective = 1;
	Serial.println("Starting");

	material_t *mpl = add_material(cont);
	//vector_set(&mpl.color, 0, 0.396, 0.7019);
	vector_set(&mpl->color, 1, 1, 1);
	mpl->defuse = 1;
	mpl->specular = 0.0;
	mpl->shine = 50;
	mpl->reflective = 0.0;


	viewpoint_init(&s->view);
	
	object_t *o = add_object(cont, TYPE_SPHERE);
	vector_set(&o->sph.center, 0, 4, 7);
	o->sph.radius = 5;
	o->m = m2;

	o = add_object(cont, TYPE_SPHERE);
	vector_set(&o->sph.center, 8, 8, 4);
	o->sph.radius = 2;
	o->m = m3;
	Serial.println("Starting");

	o = add_object(cont, TYPE_SPHERE);
	vector_set(&o->sph.center, -10, 9, 5);
	o->sph.radius = 3;
	o->m = m;

	o = add_object(cont, TYPE_SPHERE);
	vector_set(&o->sph.center, -10, -5, 5);
	o->sph.radius = 3;
	o->m = m;

	o = add_object(cont, TYPE_PLANE);
	vector_set(&o->pl.start, 0, 0, 2);
	vector_set(&o->pl.norm, 0, 0, 1);
	o->m = mpl;

	light_t *l = add_light(cont);
	vector_set(&l->pos, 20, 10, 30);
	color_set(&l->defuse, 0.3, 0.3, 0.3);
	color_set(&l->specular, 0.5, 0.5, 0.5);

	pgm_write_header(stdout, TESTW, TESTH);

	for (int y = TESTH; y; y--) {
		for (int x = TESTW; x; x--) {
			// Random seed
			color_t c;
			ray_trace(&cont->space, x, y, 2, &c, NULL);
		}
		Serial.print("row "); Serial.println(y);
	}
	
}

void loop() {
}
