#include <stdio.h>
#include <math.h>
#include "vector.h"

#include "ray.h"

// https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
// http://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection/
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
COORD_T ray_intersect_sphere(sphere_t *s, ray_t *ray, bool skip_dist)
{
    // Vector between vector start and center of circle
    vector_t oc;
    vector_sub(&oc, ray->start, s->center);

    // Solve quadratic function
	// TODO Not sure if this step i neccesary because dir is unit
    COORD_T a = vector_dot(ray->direction, ray->direction);
    COORD_T b = 2 * vector_dot(&oc, ray->direction);
    COORD_T c = vector_dot(&oc, &oc) - s->radius * s->radius;

    COORD_T d = b * b - 4 * a * c;

    // no intersection
    if (d < 0) {
        return -1;
    }
	if (skip_dist) {
		return 1;
	}

    // Else take the closest intersection, reuse d
	COORD_T q = (b > 0) ? 
		-0.5 * (b + sqrt(d)) :
		-0.5 * (b - sqrt(d));

	COORD_T x0 = q / a;
	COORD_T x1 = c / q;

	// Take the correct result. If one is zero take the other.
	if (x0 <= 0) {
		if (x1 <= 0) {
			return -1;
		}

		x0 = x1;
	}

	// If point is on sphere it will be zero close to zero
	if (x0 < 1e-3) {
		return -1;
	}

	return x0;
}

// Requires that vectors are normalized
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-plane-and-ray-disk-intersection
COORD_T ray_intersect_plane(plane_t *p, ray_t *ray, bool skip_dist)
{
	// If zero ray is parralel to plane
	COORD_T nr = vector_dot(p->norm, ray->direction);
	//
	// Take care of rounding errors
	if (nr < ZERO_APROX && nr > -ZERO_APROX) {
		return -1;
	}
	if (skip_dist) {
		return 1;
	}

	// Calculate distance
	vector_t tmp;
	vector_copy(&tmp, p->start);
	vector_sub(&tmp, &tmp, ray->start);

	COORD_T t = vector_dot(&tmp, p->norm) / nr;
	return t;
}

COORD_T ray_intersect(object_t *o, ray_t *ray, bool skip_dist)
{
	switch (o->type) {
		case TYPE_PLANE:
			return ray_intersect_plane(&o->pl, ray, skip_dist);
		case TYPE_SPHERE:
			return ray_intersect_sphere(&o->sph, ray, skip_dist);
		default:
			printf("Unknown object type %d\n", o->type);
			return -1;
	}
}

// If chk is true, will return at first hit less than chk_dist
object_t *ray_cast(space_t *s, ray_t *r, COORD_T *dist_ret, bool chk, COORD_T chk_dist)
{
	object_t *o = s->objects;

	object_t *smallest = NULL;
	COORD_T dist = 0;

	while (o) {
		COORD_T d = ray_intersect(o, r, false);

		if (d > 0) {
			if (chk && chk_dist > d) {
				if (dist_ret) {
					*dist_ret = d;
				}
				return o;
			}
			if (d < dist || smallest == NULL) {
				dist = d;
				smallest = o;
			}
		}
		
		o = o->next;
	}

	if (chk) {
		return NULL;
	}

	if (dist_ret) {
		*dist_ret = dist;
	}
	return smallest;
}

color_t *ray_trace(space_t *s, unsigned int x, unsigned int y)
{
	// Setup primary ray
	ray_t r;
	r.start = &s->view.position;
	r.direction = vector_copy(NULL, NULL);
	viewpoint_ray(&s->view, r.direction, x, y);

	// Cast it
	COORD_T dist;
	object_t *o = ray_cast(s, &r, &dist, false, 0);
	if (!o) {
		return NULL;
	}

	// Calculate new ray point
	r.start = vector_scale(NULL, r.direction, dist);
	vector_add(r.start, r.start, &s->view.position);

	// Calculate normal vector
	vector_t n;
	obj_norm_at(o, &n, r.start);

	// Hit color
	color_t *c = color_set(NULL, 0, 0, 0);

	// Cast light rays
	light_t *light = s->lights;
	while (light) {
		vector_t l;
		// Calculate distance to light
		vector_sub(&l, light->pos, r.start);
		COORD_T d = vector_len(&l);

		// Find obstacles
		r.direction = &l;
		object_t *obs = ray_cast(s, &r, NULL, true, d);
		if (obs) {
			light = light->next;
			continue;
		}
		
		// Calculate unit
		vector_scale_inv(&l, &l, vector_len(&l));
		color_t tmp;
		color_scale(&tmp, light->defuse, vector_dot(&l, &n));
		color_add(c, &tmp, c);
			
		light = light->next;
	}

	if (o->m) {
		color_scale_vector(c, c, &o->m->color);
	}

	return c;
}
