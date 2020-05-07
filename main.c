#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>

#include "vector.h"
#include "ray.h"
#include "scene.h"
#include "pgm.h"

#define TESTW 1000
#define TESTH 1000

#define PERCENTSTEP (TESTH / 100)

#define WORKERS 4

void *worker_func(void *arg);

typedef struct {
	unsigned id;
	color_t *image;
} office_t;

// Height percentage, workers take turn at printing to it. This maaaay be a bad idea
unsigned percent = 0;
pthread_mutex_t percentlock;

#define OBJECTS 10
#define MATERIALS 6
#define LIGHTS 1

char container[ CONTAINER_SIZE(OBJECTS, MATERIALS, LIGHTS) ];
container_t *cont = (container_t *) container;

settings_t gfx_high = {
    .arealight_samples = 64,
    .envlight_samples = 64,
    .antialias_samples = 2,
    .globallight_samples = 32,
    .gl_opt_depth = 1,
    .depth = 2,
};

settings_t gfx_low = {
    .arealight_samples = 16,
    .envlight_samples = 16,
    .antialias_samples = 2,
    .globallight_samples = 16,
    .gl_opt_depth = 1,
    .depth = 1,
};

settings_t gfx_very = {
    .arealight_samples = 8,
    .envlight_samples = 8,
    .antialias_samples = 2,
    .globallight_samples = 2,
    .gl_opt_depth = 0,
    .depth = 1,
};

// Implement random 
COORD_T ray_rand(void *seed)
{
	return (COORD_T) rand_r( (unsigned *)seed )  / RAND_MAX;
}

int main()
{
	container_init(cont, OBJECTS, MATERIALS, LIGHTS);
	
	// Init space_t
	space_t *s = container_prepare_space(cont);;
	
	// Set space options
	color_set(&s->ambient, 0.09, 0.09, 0.09);
	//color_set(&s->back, 0.8, 0.8, 0.8);
	color_set(&s->back, 0.0, 0.0, 0.0);
	color_set(&s->env_color, 0.13, 0.13, 0.13);
	s->env_enabled = false;
    s->gfx = &gfx_low;

	// Set viewpoint options
	vector_set(&s->view.position, 0, 5, 4);
	vector_set(&s->view.target, 0, 5, 0);
	s->view.width = TESTW;
	s->view.height = TESTH;

	// Create materials
	material_t *m = add_material(cont);
	vector_set(&m->color, 1, 1, 1);
	m->defuse = 1;
	m->specular = 0;
	m->shine = 80;
	m->reflective = 0.0;

	material_t *ml = add_material(cont);
	vector_set(&ml->color, 1, 1, 1);
	ml->emissive = 1;

	material_t *m2 = add_material(cont);
	vector_set(&m2->color, 1, 1, 1);
	m2->defuse = 0;
	m2->specular = 0.1;
	m2->shine = 1;
	m2->reflective = 1;

	material_t *mpl = add_material(cont);
	vector_set(&mpl->color, 1, 1, 1);
	mpl->defuse = 1;
	mpl->specular = 0.0;
	mpl->shine = 50;
	mpl->reflective = 0;
	
	material_t *mplgreen = add_material(cont);
	memcpy(mplgreen, mpl, sizeof(material_t));
	vector_set(&mplgreen->color, 0.3, 0.3, 1);
	//mplgreen->reflective = 1;
	//mplgreen->defuse = 0;

	material_t *mplred = add_material(cont);
	memcpy(mplred, mplgreen, sizeof(material_t));
	vector_set(&mplred->color, 1, 0.3, 0.3);

	viewpoint_init(&s->view);
	
	object_t *o;
	o = add_object(cont, TYPE_SPHERE);
	vector_set(&o->sph.center, 2, 6, -1);
	o->sph.radius = 1;
	o->m = m2;

	o = add_object(cont, TYPE_SPHERE);
	vector_set(&o->sph.center, 0, 4, -1);
	o->sph.radius = 1.3;
	o->m = m;

	o = add_object(cont, TYPE_SPHERE);
	vector_set(&o->sph.center, -2, 5, -2);
	o->sph.radius = 1.3;
	o->m = m;

	o = add_object(cont, TYPE_PLANE);
	vector_set(&o->pl.start, 0, 0, 0);
	vector_set(&o->pl.norm, 0, 1, 0);
	o->m = mpl;

	o = add_object(cont, TYPE_PLANE);
	vector_set(&o->pl.start, 0, 10, 0);
	vector_set(&o->pl.norm, 0, 1, 0);
	o->m = mpl;

	o = add_object(cont, TYPE_PLANE);
	vector_set(&o->pl.start, 0, 0, -5);
	vector_set(&o->pl.norm, 0, 0, 1);
	o->m = mpl;

	o = add_object(cont, TYPE_PLANE);
	vector_set(&o->pl.start, 0, 0, 10);
	vector_set(&o->pl.norm, 0, 0, 1);
	o->m = mpl;

	o = add_object(cont, TYPE_PLANE);
	vector_set(&o->pl.start, -5, 0, 0);
	vector_set(&o->pl.norm, 1, 0, 0);
	o->m = mplgreen;

	o = add_object(cont, TYPE_PLANE);
	vector_set(&o->pl.start, 5, 0, 0);
	vector_set(&o->pl.norm, 1, 0, 0);
	o->m = mplred;
	
	// Used for the light ball
	o = add_object(cont, TYPE_SPHERE);
	vector_set(&o->sph.center, 0, 7, 0);
	o->sph.radius = 0.5;
	o->m = ml;

	light_t *l = add_light(cont, TYPE_L_AREA);
	//vector_set(&l->point.pos, 0, 5, 0);
	l->area = o;
	color_set(&l->color, 1, 1, 1);
	l->radiance = 0.1;

	pgm_write_header(stdout, TESTW, TESTH);

	// Create image array. Not as memory efficient but much simpler when multiprocessing
	color_t *image = malloc( sizeof(color_t) * TESTW * TESTH);
	if (!image) {
		fprintf(stderr, "Could not allocate image array");	
		exit(1);
	}

	if (pthread_mutex_init(&percentlock, NULL)) {
		fprintf(stderr, "Could not percent lock\n");
		exit(1);
	}

	// Hire the workers
	pthread_t workers[WORKERS];
	for (int i = 0; i < WORKERS; i++) {
		office_t *office = malloc(sizeof(office_t));
		office->id = i;
		office->image = image;

		// Start them and show them their office(chunk in the array in this case :-D)
		int rc = pthread_create(&workers[i], NULL, worker_func, office);
		if (rc) {
			fprintf(stderr, "Could not create worker %d\nsorry\n", i);
			exit(1);
		}
	}

	// Wait for the threads to finish and print as we go
	for (int i = 0; i < WORKERS; i++) {
		pthread_join(workers[i], NULL);
		
	}

	// Print the stuff the worker was responsable for.
	for (int y = 0; y < TESTH; y++) {
		for(int x = 0; x < TESTW; x++) {
			pgm_write_pixel(stdout, &image[ y  * TESTW + x]);
		}
	}

	free(image);
	
}

void *worker_func(void *arg) {
	// Organize our office
	office_t *office = (office_t *) arg;

	int seed;

	// Start working
	for (int y = office->id; y < TESTH; y += WORKERS) {
		// Start time
		for (int x = 0; x < TESTW; x++) {
			color_t *c = &office->image[ y * TESTW + x ];
			//color_t c;
			seed = x * y;
			ray_trace(&cont->space, TESTW - x, TESTH - y, c, &seed);
			color_clamp(c);
		}

		if (y % PERCENTSTEP == 0) {
			// Unlock the thingy
			pthread_mutex_lock(&percentlock);	
			fprintf(stderr, "%d%\n", percent++);
			pthread_mutex_unlock(&percentlock);	
		}
	}

	//free(office);
	pthread_exit(NULL);
}
