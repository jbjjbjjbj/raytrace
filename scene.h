
#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>
#include <stdint.h>
#include "vector.h"
#include "viewpoint.h"
#include "pgm.h"

#define TYPE_SPHERE 1
#define TYPE_PLANE 2

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

	struct light_s *next;
} light_t;

typedef struct {
	vector_t color;
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
} space_t;

object_t *add_sphere(space_t *s, vector_t *c, COORD_T r, material_t *m);
object_t *add_plane(space_t *s, vector_t *start, vector_t *dir);
light_t *add_light(space_t *s, vector_t *pos, color_t *defuse);

void obj_norm_at(object_t *o, vector_t *dest, vector_t *point);

#endif
