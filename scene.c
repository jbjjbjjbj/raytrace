#include "scene.h"

#include <stdlib.h>
#include <string.h>

static inline void link_object(space_t *s, object_t *o)
{
	if (s) {
		o->next = s->objects;
		s->objects = o;
	} else {
		o->next = 0;
	}
}

container_t *container_init(container_t *c, unsigned objs, unsigned mats, unsigned ligs) 
{
	c->obj_index = c->mat_index = c->lig_index = 0;

	c->obj_cap = objs;
	c->mat_cap = mats;
	c->lig_cap = ligs;

	return c;
}

space_t *container_prepare_space(container_t *c)
{
	memset(&c->space, 0, sizeof(space_t));
}

// Finds the next empty object_t space
// Return NULL if full
static inline object_t *container_obj_space(container_t *cont) 
{
	if (cont->obj_index >= cont->obj_cap) {
		fprintf(stderr, "Could not create object, because container is full\n");
		return NULL;
	}

	// Find a nice spot in the content blob
	return &((object_t *) cont->content)[cont->obj_index++];
}

static inline material_t *container_mat_space(container_t *cont) 
{
	if (cont->mat_index >= cont->mat_cap) {
		fprintf(stderr, "Could not create material, because container is full\n");
		return NULL;
	}

	// Find a nice spot in the content blob.
	// Remember to jump over objects
	return &((material_t *) (
				cont->content + 
				sizeof(object_t) * cont->obj_cap
				))[cont->mat_index++];
}

static inline light_t *container_lig_space(container_t *cont) 
{
	if (cont->lig_index >= cont->lig_cap) {
		fprintf(stderr, "Could not create light, because container is full\n");
		return NULL;
	}

	// Find a nice spot in the content blob.
	// Remember to jump over objects and materials
	return &((light_t *) (
				cont->content + 
				sizeof(object_t) * cont->obj_cap + 
				sizeof(material_t) * cont->mat_cap
				))[cont->lig_index++];
}

object_t *add_sphere(container_t *cont, vector_t *c, COORD_T r, material_t *m)
{

	object_t *o = container_obj_space(cont);
	if (!o) {
		return NULL;
	}

	// Fill out the data
	o->type = TYPE_SPHERE;
	o->m = m;

	o->sph.center = c;
	o->sph.radius = r;

	// Link to the linked list
	link_object(&cont->space, o);

	return o;
}

object_t *add_plane(container_t *cont, vector_t *start, vector_t *dir, material_t *m)
{

	object_t *o = container_obj_space(cont);
	if (!o) {
		return NULL;
	}

	o->type = TYPE_PLANE;
	o->m = m;

	o->pl.start = start;
	o->pl.norm = dir;

	link_object(&cont->space, o);

	return o;
}

light_t *add_light(container_t *cont, vector_t *pos, color_t *defuse, color_t *specular)
{
	light_t *o = container_lig_space(cont);
	if (!o) {
		return NULL;
	}

	o->pos = pos;
	o->defuse = defuse;
	o->specular = specular;

	space_t *s = &cont->space;
	if (s) {
		o->next = s->lights;
		s->lights = o;
	} else {
		o->next = NULL;
	}

	return o;
}

// It's alot easier to set the stuff outside of this function
material_t *add_material(container_t *cont)
{
	material_t *m = container_mat_space(cont);

	return m;
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
