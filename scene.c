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

object_t *add_sphere(space_t *s, vector_t *c, COORD_T r, material_t *m)
{
	object_t *o = (object_t *) malloc(sizeof(object_t));

	o->type = TYPE_SPHERE;
	o->m = m;

	o->sph.center = c;
	o->sph.radius = r;

	link_object(s, o);

	return o;
}

object_t *add_plane(space_t *s, vector_t *start, vector_t *dir, material_t *m)
{
	object_t *o = (object_t *) malloc(sizeof(object_t));

	o->type = TYPE_PLANE;
	o->m = m;

	o->pl.start = start;
	o->pl.norm = dir;

	link_object(s, o);

	return o;
}

light_t *add_light(space_t *s, vector_t *pos, color_t *defuse, color_t *specular)
{
	light_t *o = (light_t *) malloc(sizeof(light_t));

	o->pos = pos;
	o->defuse = defuse;
	o->specular = specular;

	if (s) {
		o->next = s->lights;
		s->lights = o;
	} else {
		o->next = NULL;
	}

	return o;
}

void obj_norm_at(object_t *o, vector_t *dest, vector_t *point)
{
	switch(o->type) {
		case TYPE_SPHERE:
			vector_sub(dest, point, o->sph.center);
			vector_scale_inv(dest, dest, vector_len(dest));
			break;
		case TYPE_PLANE:
			vector_copy(dest, o->pl.norm);
			break;
	}

	return;
}
