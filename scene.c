#include "scene.h"

#include <stdlib.h>

static inline void link_object(space_t *s, object_t *o)
{
	if (s) {
		o->next = s->objects;
		s->objects = o;
	} else {
		o->next = 0;
	}
}

object_t *add_sphere(space_t *s, vector_t *c, COORD_T r)
{
	object_t *o = (object_t *) malloc(sizeof(object_t));

	o->type = TYPE_SPHERE;
	o->sph.center = c;
	o->sph.radius = r;

	link_object(s, o);

	return o;
}

object_t *add_plane(space_t *s, vector_t *start, vector_t *dir)
{
	object_t *o = (object_t *) malloc(sizeof(object_t));

	o->type = TYPE_PLANE;
	o->pl.start = start;
	o->pl.norm = dir;

	link_object(s, o);

	return o;
}

light_t *add_light(space_t *s, vector_t *pos, color_t *c)
{
	light_t *o = (light_t *) malloc(sizeof(light_t));

	o->pos = pos;
	o->col = c;

	if (s) {
		o->next = s->lights;
		s->lights = o;
	} else {
		o->next = NULL;
	}

	return o;
}
