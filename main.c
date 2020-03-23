#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>

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

char container[ CONTAINER_SIZE(5, 4, 1) ];
container_t *cont = (container_t *) container;

// Implement random 
COORD_T ray_rand(void *seed)
{
	return (COORD_T) rand_r( (int *)seed )  / RAND_MAX;
}

int main()
{
	container_init(cont, 5, 4, 1);
	
	// Init space_t
	space_t *s = container_prepare_space(cont);;
	
	// Set space options
	color_set(&s->ambient, 0.09, 0.09, 0.09);
	color_set(&s->back, 0.8, 0.8, 0.8);
	color_set(&s->env_color, 0.13, 0.13, 0.13);
	s->env_samples = 16;

	// Set viewpoint options
	vector_set(&s->view.position, 0, 16, 6);
	vector_set(&s->view.target, 0, 0, 6);
	s->view.width = TESTW;
	s->view.height = TESTH;

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

	material_t *m2 = add_material(cont);
	vector_set(&m2->color, 1, 1, 1);
	m2->defuse = 0.0;
	m2->specular = 0.5;
	m2->shine = 80;
	m2->reflective = 1;

	material_t *mpl = add_material(cont);
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
			ray_trace(&cont->space, TESTW - x, TESTH - y, 2, c, &seed);
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
