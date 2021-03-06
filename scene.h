
#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "viewpoint.h"
#include "pgm.h"

#define TYPE_SPHERE 1
#define TYPE_PLANE 2

#define TYPE_L_POINT 1
#define TYPE_L_AREA 2

#define CONTAINER_SIZE(objs, mats, ligs) (sizeof(container_t) + objs * sizeof(object_t) + ligs * sizeof(light_t))

typedef struct {
	vector_t center;
	COORD_T radius;
} sphere_t;

typedef struct {
	vector_t start;
	vector_t norm;
} plane_t;

typedef struct {
	vector_t color;

	// Light properties
	COORD_T defuse;
	// If this object emits light. Only makes sense if combined with a area light.
	COORD_T emissive; 
	COORD_T specular;
	unsigned int shine;
	
	// Reflective properties
	COORD_T reflective;
} material_t;

// General object structure
typedef struct object_s{
	unsigned type;
	struct object_s *next;

	// Color and light information
	material_t *m;

	union {
		sphere_t sph;
		plane_t pl;
	};
} object_t;

typedef struct light_s{
	unsigned type;
	color_t color;

	COORD_T radiance;

	union {
		struct {
			vector_t pos;
		} point;
		object_t *area;
	};
	struct light_s *next;
} light_t;

light_t l;

// Graphics settings
// Should actually sit in ray.c
typedef struct {
    unsigned arealight_samples;
    unsigned antialias_samples;
    unsigned envlight_samples;
    unsigned globallight_samples;

    unsigned depth;
    unsigned gl_opt_depth;
} settings_t;

typedef struct {
	viewpoint_t view;
	
	object_t *objects;
	light_t *lights;

	color_t ambient;
	color_t back;

    settings_t *gfx;
	// Environment light
	// TODO make more general
	// Slows things down alot
    bool env_enabled;
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

object_t *add_object(container_t *cont, unsigned type);
light_t *add_light(container_t *cont, unsigned type);
material_t *add_material(container_t *cont);

void obj_norm_at(object_t *o, vector_t *dest, vector_t *point, vector_t *direction);

#endif
