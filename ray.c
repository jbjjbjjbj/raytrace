#include <stdio.h>
#include <math.h>
#include <stdlib.h>
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

	COORD_T x1 = q / a;
	COORD_T x0 = c / q;

	// Take the correct result. If one is zero take the other.
	if (x0 <= ZERO_APROX) {
		if (x1 <= 0) {
			return -1;
		}

		x0 = x1;
	}

	// If point is on sphere it will be zero close to zero
	if (x0 < ZERO_APROX) {
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

		if (d > ZERO_APROX) {
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

static void ray_calc_light(space_t *s, color_t *dest, object_t *o, vector_t *N, vector_t *eye, vector_t *point)
{
	ray_t r;
	r.start = point;
	
	// And vector towards viewer
	vector_t V;
	vector_sub(&V, eye, point);

	// Normalice it
	vector_scale_inv(&V, &V, vector_len(&V));

	// Cast light rays
	light_t *light = s->lights;
	while (light) {
		vector_t l;
		
		// Calculate distance to light
		vector_sub(&l, light->pos, point);
		COORD_T d = vector_len(&l);

		// Normalice
		vector_scale_inv(&l, &l, vector_len(&l));

		// Find obstacles
		r.direction = &l;
		object_t *obs = ray_cast(s, &r, NULL, true, d);
		if (obs) {
			light = light->next;
			continue;
		}
		
		// Calculate Deffuse part
		color_t tmp;
		COORD_T cl = vector_dot(&l, N);
		if (cl > 0) {
			color_scale(&tmp, light->defuse, cl * o->m->defuse);
			color_add(dest, &tmp, dest);
		}

		// calculate specular part. TODO implement blinn-phong
		// Calculate R_m
		vector_t R;
		vector_scale(&R, N, 2 * vector_dot(&l, N));
		vector_sub(&R, &R, &l);
		
		// Add it to the light
		cl = 1 * vector_dot(&R, &V);
		if (cl > 0) {
			cl = pow(cl, o->m->shine);
			color_scale(&tmp, light->specular, cl * o->m->specular);
			color_add(dest, &tmp, dest);
		}

		light = light->next;
	}
}

int ray_trace_recur(space_t *s, color_t *dest, ray_t *ray, unsigned hop, COORD_T scale)
{
	COORD_T dist;
	color_t c;
	color_set(&c, 0, 0, 0);

	object_t *o = ray_cast(s, ray, &dist, false, 0);
	if (!o) {
		color_add(&c, &c, &s->back);
		goto exit;
	}

	vector_t rdir, rstart;
	ray_t r = {start: &rstart, direction: &rdir};

	vector_scale(r.start, ray->direction, dist);
	vector_add(r.start, r.start, ray->start);

	// Calculate normal vector
	vector_t N;
	obj_norm_at(o, &N, r.start);

	// Check if we should calculate light
	if (o->m->defuse + o->m->specular > ZERO_APROX) {
		// Add all light hitting o at r.start to c
		ray_calc_light(s, &c, o, &N, ray->start, r.start);
	}

	// Calculate reflection vector
	if (hop < 2 && o->m->reflective > ZERO_APROX) {
		vector_scale(r.direction, &N, 2 * vector_dot(ray->direction, &N));
		vector_sub(r.direction, ray->direction, r.direction);

		ray_trace_recur(s, &c, &r, hop+1, o->m->reflective);
	}


	// Scale by the objects own color.
	color_scale_vector(&c, &c, &o->m->color);

exit:
	// Add it to the result
	color_scale(&c, &c, scale);
	color_add(dest, dest, &c);

	return 0;
}

color_t *ray_trace(space_t *s, unsigned int x, unsigned int y, unsigned samples)
{
	// Init return color. Will be accumilated with all the detected light.
	color_t *c = color_set(NULL, 0, 0, 0);

	// Setup primary ray
	ray_t r;
	r.start = &s->view.position;
	r.direction = vector_copy(NULL, NULL);

	// Multiple samples for antialias
	// TODO better distribution of antialias probes
	for (int i = 0; i < samples; i++) {
		color_t ctmp;
		color_set(&ctmp, 0, 0, 0);
		//memset(&ctmp, 0, sizeof(color_t));
		
		// Multiple samples inside same pixel
		COORD_T tmp = (COORD_T) i/ (COORD_T) samples;
		viewpoint_ray(&s->view, r.direction, x + tmp, y + tmp);

		// Run the recursive ray trace
		ray_trace_recur(s, &ctmp, &r, 0, 1);

		// Color_add will not go above 1. In this case we don't want that.
		c->r += ctmp.r; c->g += ctmp.g; c->b += ctmp.b;

	}
	//printf("COlor before devide %f, %f, %f\n", c->r, c->g, c->b);

	free(r.direction);

	// Take the median
	if (samples > 1) {
		// Same as deviding by samples
		color_scale(c, c, 1.0/ (COORD_T) samples);
	}

	// Add ambient
	color_add(c, c, &s->ambient);

	return c;
}
