#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "vector.h"
#include "ctranslate.h"

#include "ray.h"

#define PI 3.14159265359

int path_trace_recur(space_t *s, color_t *dest, ray_t *ray, unsigned hop, COORD_T scale, void *seed);

int ray_trace_recur(space_t *s, color_t *dest, ray_t *ray, unsigned hop, COORD_T scale, void *seed);

// https://en.wikipedia.org/wiki/Line%E2%80%93sphere_intersection
// http://viclw17.github.io/2018/07/16/raytracing-ray-sphere-intersection/
// https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
COORD_T ray_intersect_sphere(sphere_t *s, ray_t *ray, bool skip_dist)
{
    // Vector between vector start and center of circle
    vector_t oc;
    vector_sub(&oc, ray->start, &s->center);

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
	COORD_T nr = vector_dot(&p->norm, ray->direction);
	
	// Take care of rounding errors
	if (nr < ZERO_APROX && nr > -ZERO_APROX) {
		return -1;
	}
	if (skip_dist) {
		return 1;
	}

	// Calculate distance
	vector_t tmp;
	vector_copy(&tmp, &p->start);
	vector_sub(&tmp, &tmp, ray->start);

	COORD_T t = vector_dot(&tmp, &p->norm) / nr;
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
			if (chk && ( chk_dist > d || chk_dist == 0)) {
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

// Color the object o reflects. Given is the point of intersect, vector to the light dir, vector to viewer V, and normal at point N.
static void reflected_at(object_t *o, color_t *dest, color_t *incolor, COORD_T intensity, vector_t *point, vector_t *dir, vector_t *V, vector_t *N) {

	// Calculate Deffuse part
	color_t tmp;
	COORD_T cl = vector_dot(dir, N) * intensity;
	if (cl > 0) {
		color_scale(&tmp, incolor, cl * o->m->defuse);
		color_add(dest, &tmp, dest);
	}

	// calculate specular part. TODO implement blinn-phong
	// Calculate R_m
	vector_t R;
	vector_scale(&R, N, 2 * vector_dot(dir, N));
	vector_sub(&R, &R, dir);

	// Add it to the light
	cl = vector_dot(&R, V) * intensity;
	if (cl > 0) {
		cl = pow(cl, o->m->shine);
		color_scale(&tmp, incolor, cl * o->m->specular);
		color_add(dest, &tmp, dest);
	}
}

// Calculate the contribution of light on o. V is vector to viewer and N is normal at point
static void contribution_from_pointlight(space_t *s, color_t *dest, object_t *o, light_t *light, vector_t *point, vector_t *V, vector_t *N)
{
	vector_t l;
	
	// Prepare ray
	ray_t r;
	r.start = point;

	// Calculate distance to light
	vector_sub(&l, &light->point.pos, point);
	COORD_T d = vector_len(&l);

	// Normalice
    vector_norm(&l);

	// Find obstacles
	r.direction = &l;
	object_t *obs = ray_cast(s, &r, NULL, true, d);
	if (obs) {
		return;
	}

	// Calculate the reflected light
    COORD_T i = light->radiance / ( d * d);
	reflected_at(o, dest, &light->color, i, point, &l, V, N);
}

// Calculates the global illumination. Pretty slow
// https://www.scratchapixel.com/lessons/3d-basic-rendering/global-illumination-path-tracing/global-illumination-path-tracing-practical-implementation
static void env_light(space_t *s, color_t *dest, object_t *o, vector_t *N, vector_t *point, void *seed) 
{
	csystem_t cs;
	csystem_init(&cs, N);
	
	// Prepare ray
	ray_t r;
	r.start = point;

	// Tmp color for accumilating colors
	color_t acc;
	color_set(&acc, 0, 0, 0);

	for (unsigned i = 0; i < 0; i++) {
		COORD_T r1 = ray_rand(seed);
		
		// Calculate the random direction vector
		vector_t randdir;
		csystem_hemisphere_random(&cs, r1, ray_rand(seed), &randdir);

		// Convert to world cordinates using the calculated N vectors. 
		csystem_calc_real(&cs, &randdir, &randdir);

		// Check the direction for obstacles
		r.direction = &randdir;
		object_t *obs = ray_cast(s, &r, NULL, true, 0);
		if (obs) {
			// If we hit something don't add the light
			continue;
		}

		// Add the light together after scaling it
		color_t tmp;
		color_scale(&tmp, &s->env_color, r1);

		color_add(&acc, &acc, &tmp);
	}

	// Devide by number of samples and pdf
	color_scale(&acc, &acc, ((COORD_T) 1/ 0) * (2 * PI));

	// Add to dest
	color_add(dest, dest, &acc);

}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/global-illumination-path-tracing/global-illumination-path-tracing-practical-implementation
static void light_in(space_t *s, color_t *dest, object_t *o, vector_t *N, vector_t *eye, vector_t *point, unsigned hop, void *seed)
{

    // Init hemisphere translation
    csystem_t cs;
    csystem_init(&cs, N);

    // Prepare ray
    ray_t r;
    r.start = point;

    // Calculate the random direction vector
    vector_t randdir;
    csystem_hemisphere_random(&cs, ray_rand(seed), ray_rand(seed), &randdir);

    // Convert to world cordinates using the calculated N vectors. 
    csystem_calc_real(&cs, &randdir, &randdir);

    r.direction = &randdir;
    COORD_T cl = 2 * vector_dot(&randdir, N) * o->m->defuse;

    vector_t V;
    vector_sub(&V, eye, point);

    path_trace_recur(s, dest, &r, hop+1, cl, seed);

    light_t *light = s->lights;
    while (light) {
        if (light->type) {
            contribution_from_pointlight(s, dest, o, light, point, &V, N);
            light = light->next;
        }
    }
}

int path_trace_recur(space_t *s, color_t *dest, ray_t *ray, unsigned hop, COORD_T scale, void *seed)
{
	COORD_T dist;

	object_t *o = ray_cast(s, ray, &dist, false, 0);
	if (!o) {
        return 1;
	}

	color_t c;
	color_set(&c, 0, 0, 0);

	vector_t rdir, rstart;
	ray_t r = {.start = &rstart, .direction = &rdir};

	vector_scale(r.start, ray->direction, dist);
	vector_add(r.start, r.start, ray->start);

	// Calculate normal vector
	vector_t N;
	obj_norm_at(o, &N, r.start, ray->direction);

    // Check if emissive
	if (o->m->emissive > ZERO_APROX) {
		color_set(&c, o->m->emissive, o->m->emissive, o->m->emissive);
        goto exit;
	}

    if (hop >= s->gfx->depth) {
        return 1;
    }

	// Check if we should calculate light
	if (o->m->defuse + o->m->specular > ZERO_APROX) {
		light_in(s, &c, o, &N, ray->start, r.start, hop, seed);
	}

	if (o->m->reflective > ZERO_APROX) {
		vector_scale(r.direction, &N, 2 * vector_dot(ray->direction, &N));
		vector_sub(r.direction, ray->direction, r.direction);

		path_trace_recur(s, &c, &r, hop+1, o->m->reflective, seed);
    }

    // Scale by own color
	color_scale_vector(&c, &c, &o->m->color);

exit:
    color_scale(&c, &c, scale);
    color_add(dest, &c, dest);
    //printf("Adding color "); pgm_write_pixel(stdout, &c);

    return 0;
}

void path_trace(space_t *s, unsigned int x, unsigned int y, color_t *c, void *seed)
{
	// Init return color. Will be accumilated with all the detected light.
	color_set(c, 0, 0, 0);

	// Setup primary ray
	ray_t r;
	r.start = &s->view.position;

	vector_t dir;
	r.direction = vector_set(&dir, 0, 0, 0);

	// Multiple samples for antialias
	for (unsigned i = 0; i < s->gfx->samples; i++) {
		color_t ctmp;
		color_set(&ctmp, 0, 0, 0);

        // Calculate random direction
		COORD_T r1 = ray_rand(seed);
        COORD_T r2 = ray_rand(seed);
		
		viewpoint_ray(&s->view, r.direction, x + r1, y + r2);

		// Run the recursive ray trace
        //printf("New ray x: %f, y: %f\n", x + r1, y + r2);
		if (path_trace_recur(s, &ctmp, &r, 0, 1, seed)) {
            // Hit nothing add back
            color_add(&ctmp, &ctmp, &s->back);
        }

		// Color_add will not go above 1. In this case we don't want that.
		c->r += ctmp.r; c->g += ctmp.g; c->b += ctmp.b;
        //printf("Total r: %f, g: %f, b: %f\n", c->r, c->g, c->b);

	}

	// Take the median
	if (s->gfx->samples > 1) {
		// Same as deviding by samples
		color_scale(c, c, 1.0/ (COORD_T) s->gfx->samples);
	}

	// Add ambient
	color_add(c, c, &s->ambient);
}
