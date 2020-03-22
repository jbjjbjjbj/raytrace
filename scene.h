
#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "viewpoint.h"
#include "pgm.h"

#define TYPE_SPHERE 1
#define TYPE_PLANE 2

#define CONTAINER_SIZE(objs, mats, ligs) (sizeof(container_t) + objs * sizeof(object_t) + ligs * sizeof(light_t))

typedef struct {
	vector_t *center;
	COORD_T radius;
} sphere_t;

typedef struct {
	vector_t *start;
	vector_t *norm;
} plane_t;

typedef struct light_s{
	vector_t *pos;
	color_t *defuse;
	color_t *specular;

	struct light_s *next;
} light_t;

typedef struct {
	vector_t color;

	// Light properties
	COORD_T defuse;
	COORD_T specular;
	unsigned int shine;
	
	// Reflective properties
	COORD_T reflective;
} material_t;

// General object structure
typedef struct object_s{
	uint8_t type;
	struct object_s *next;

	// Color and light information
	material_t *m;

	union {
		sphere_t sph;
		plane_t pl;
	};
} object_t;

typedef struct {
	viewpoint_t view;
	
	object_t *objects;
	light_t *lights;

	color_t ambient;
	color_t back;

	// Environment light
	// TODO make more general
	// Slows things down alot
	unsigned env_samples;
	color_t env_color;
} space_t;

// Contains materials, space, objects and lights. Made to keep it all together.
// They will still be linked list, but close together.
typedef struct {
	space_t space;

	// Number of objects
	unsigned obj_index;
	unsigned mat_index;
	unsigned lig_index;

	// Number of objects that will fit.
	unsigned obj_cap;
	unsigned mat_cap;
	unsigned lig_cap;

	// Memory blob
	// Contains objects -> materials -> lights
	uint8_t content[0];
} container_t;


// Initialice container in buffer c. Should be big enough for the whole scene.
// Use the container_size to calculate the needed space.
// Leave c at NULL to allocate container
container_t *container_init(container_t *c, unsigned objs, unsigned mats, unsigned ligs);

space_t *container_prepare_space(container_t *c);

object_t *add_sphere(container_t *cont, vector_t *c, COORD_T r, material_t *m);
object_t *add_plane(container_t *cont, vector_t *start, vector_t *dir, material_t *m);
light_t *add_light(container_t *cont, vector_t *pos, color_t *defuse, color_t *specular);
material_t *add_material(container_t *cont);

void obj_norm_at(object_t *o, vector_t *dest, vector_t *point);

#endif
