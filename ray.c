#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <assert.h>
#include "vector.h"
#include "ctranslate.h"

#include "ray.h"

#define PI 3.14159265359

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

// Light (l) the object o reflects. Given is the point of intersect, vector to the light l, vector to viewer V, and normal at point N.
static void reflected_at(object_t *o, color_t *dest, light_t *light, COORD_T dist, vector_t *point, vector_t *l, vector_t *V, vector_t *N) {
	// Calculate light intensity
	COORD_T i = light->radiance / ( dist * dist);

	// Calculate Deffuse part
	color_t tmp;
	COORD_T cl = vector_dot(l, N) * i;
	if (cl > 0) {
		color_scale(&tmp, &light->color, cl * o->m->defuse);
		color_add(dest, &tmp, dest);
	}

	// calculate specular part. TODO implement blinn-phong
	// Calculate R_m
	vector_t R;
	vector_scale(&R, N, 2 * vector_dot(l, N));
	vector_sub(&R, &R, l);

	// Add it to the light
	cl = vector_dot(&R, V) * i;
	if (cl > 0) {
		cl = pow(cl, o->m->shine);
		color_scale(&tmp, &light->color, cl * o->m->specular);
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
	vector_scale_inv(&l, &l, vector_len(&l));

	// Find obstacles
	r.direction = &l;
	object_t *obs = ray_cast(s, &r, NULL, true, d);
	if (obs) {
		return;
	}

	// Calculate the reflected light
	reflected_at(o, dest, light, d, point, &l, V, N);
}

// Many of these can maybe be put in a context struct
static void contribution_from_arealight(space_t *s, color_t *dest, object_t *o, light_t *light, vector_t *point, vector_t *V, vector_t *N, void *seed)
{
	// This only works with spheres
	assert(light->area->type == TYPE_SPHERE);

	// Color to collect temporary results in
	color_t c;
	color_set(&c, 0, 0, 0);

	ray_t ray;
	ray.start = point;

	// Calculate vector from light to point
	vector_t l;
	vector_sub(&l, point, &light->area->sph.center);
	vector_scale_inv(&l, &l, vector_len(&l));

	// Initialize the transformation stuff
	csystem_t cs;
	csystem_init(&cs, &l);

	// Do the same monte carlo as with environment but the starting point is the center of the circle.
	// And the result is a point on the circle
	for (int i = 0; i < s->gfx->arealight_samples; i++) {
		// Do the monte carlo random distribution thing from the article
		COORD_T r1 = ray_rand(seed);

		// Random direction on halv sphere pointing towards point
		vector_t randpoint;
		csystem_hemisphere_random(&cs, r1, ray_rand(seed), &randpoint);
		csystem_calc_real(&cs, &randpoint, &randpoint);
		
		// Shift it up to center of circle
		vector_add(&randpoint, &randpoint, &light->area->sph.center);

		// Cast a ray towards it, reuse randpoint as direction
		vector_sub(&randpoint, &randpoint, point);
		COORD_T dist = vector_len(&randpoint);

		vector_t dir;
		vector_scale_inv(&dir, &randpoint, dist);

		ray.direction = &dir;

		object_t *obs = ray_cast(s, &ray, NULL, true, dist - ZERO_APROX);
		if (obs) {
			// We hit something skip it.
			continue;
		}

		// Add the light contribution, Not sure why it is scaled
		reflected_at(o, &c, light, dist, point, &randpoint, V, N);

	}

	// Device by pdf
	color_scale(&c, &c, ((COORD_T) 1 / s->gfx->arealight_samples) * (2 * PI));

	color_add(dest, dest, &c);

}

static void direct_light(space_t *s, color_t *dest, object_t *o, vector_t *N, vector_t *eye, vector_t *point, void *seed)
{
	// And vector towards viewer
	vector_t V;
	vector_sub(&V, eye, point);

	// Normalice it
	vector_scale_inv(&V, &V, vector_len(&V));

	// Loop lights
	light_t *light = s->lights;
	while (light) {
		// Calculate contribution depending on the light type
		switch (light->type) {
			case TYPE_L_POINT:
				contribution_from_pointlight(s, dest, o, light, point, &V, N);
				break;
			case TYPE_L_AREA:
				contribution_from_arealight(s, dest, o, light, point, &V, N, seed);
				break;
		}

		light = light->next;
	}
}

// Calculates the global illumination. Pretty slow
// https://www.scratchapixel.com/lessons/3d-basic-rendering/global-illumination-path-tracing/global-illumination-path-tracing-practical-implementation
static void env_light(space_t *s, color_t *dest, object_t *o, vector_t *N, vector_t *point, void *seed) 
{
    if (s->gfx->envlight_samples == 0) {
        return;
    }
	csystem_t cs;
	csystem_init(&cs, N);
	
	// Prepare ray
	ray_t r;
	r.start = point;

	// Tmp color for accumilating colors
	color_t acc;
	color_set(&acc, 0, 0, 0);

	for (unsigned i = 0; i < s->gfx->envlight_samples; i++) {
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
	color_scale(&acc, &acc, ((COORD_T) 1/ s->gfx->envlight_samples) * (2 * PI));

	// Add to dest
	color_add(dest, dest, &acc);

}

// https://www.scratchapixel.com/lessons/3d-basic-rendering/global-illumination-path-tracing/global-illumination-path-tracing-practical-implementation
static void global_light(space_t *s, color_t *dest, object_t *o, vector_t *N, vector_t *point, unsigned hop, void *seed)
{
    if (s->gfx->globallight_samples == 0) {
        return;
    }

    // Init hemisphere translation
    csystem_t cs;
    csystem_init(&cs, N);

    // Prepare ray
    ray_t r;
    r.start = point;

    // Value for accumilating colors
    color_t acc;
    color_set(&acc, 0, 0, 0);
    
    for (unsigned i = 0; i < s->gfx->globallight_samples; i++) {
		COORD_T r1 = ray_rand(seed);
		
		// Calculate the random direction vector
		vector_t randdir;
		csystem_hemisphere_random(&cs, r1, ray_rand(seed), &randdir);

		// Convert to world cordinates using the calculated N vectors. 
		csystem_calc_real(&cs, &randdir, &randdir);

		// Check the direction for obstacles
		r.direction = &randdir;

        // Cast ray in direction if we have more hops
        if (hop < s->gfx->depth) {
            ray_trace_recur(s, &acc, &r, hop+1, r1, seed);
        }
    }

    // Devide by number of samples and pdf
	color_scale(&acc, &acc, ((COORD_T) 1/ s->gfx->globallight_samples) * (2 * PI));

	// Add to dest
	color_add(dest, dest, &acc);
}

int ray_trace_recur(space_t *s, color_t *dest, ray_t *ray, unsigned hop, COORD_T scale, void *seed)
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
	ray_t r = {.start = &rstart, .direction = &rdir};

	vector_scale(r.start, ray->direction, dist);
	vector_add(r.start, r.start, ray->start);

	// Calculate normal vector
	vector_t N;
	obj_norm_at(o, &N, r.start, ray->direction);

	// Check if emissive
	if (o->m->emissive > ZERO_APROX) {
		color_set(&c, o->m->emissive, o->m->emissive, o->m->emissive);
	}

	// Check if we should calculate light
	if (o->m->defuse + o->m->specular > ZERO_APROX) {
		// Add all light hitting o at r.start to c
		direct_light(s, &c, o, &N, ray->start, r.start, seed);
	}
	
	// Calculate environmental light
	if (s->env_enabled) {
		env_light(s, &c, o, &N, r.start, seed);
	}

    global_light(s, &c, o, &N, r.start, hop, seed);

	// Calculate reflection vector
	if (hop < 10 && o->m->reflective > ZERO_APROX) {
		vector_scale(r.direction, &N, 2 * vector_dot(ray->direction, &N));
		vector_sub(r.direction, ray->direction, r.direction);

		ray_trace_recur(s, &c, &r, hop+1, o->m->reflective, seed);
	}


	// Scale by the objects own color.
	color_scale_vector(&c, &c, &o->m->color);

exit:
	// Add it to the result
	color_scale(&c, &c, scale);
	color_add(dest, dest, &c);

	return 0;
}

void ray_trace(space_t *s, unsigned int x, unsigned int y, color_t *c, void *seed)
{
	// Init return color. Will be accumilated with all the detected light.
	color_set(c, 0, 0, 0);

	// Setup primary ray
	ray_t r;
	r.start = &s->view.position;

	vector_t dir;
	r.direction = vector_set(&dir, 0, 0, 0);

	// Multiple samples for antialias
	// TODO better distribution of antialias probes
	for (int i = 0; i < s->gfx->antialias_samples; i++) {
		color_t ctmp;
		color_set(&ctmp, 0, 0, 0);
		//memset(&ctmp, 0, sizeof(color_t));
		
		// Multiple samples inside same pixel
		COORD_T tmp = (COORD_T) i/ (COORD_T) s->gfx->antialias_samples;
		viewpoint_ray(&s->view, r.direction, x + tmp, y + tmp);

		// Run the recursive ray trace
		ray_trace_recur(s, &ctmp, &r, 0, 1, seed);

		// Color_add will not go above 1. In this case we don't want that.
		c->r += ctmp.r; c->g += ctmp.g; c->b += ctmp.b;

	}

	// Take the median
	if (s->gfx->antialias_samples > 1) {
		// Same as deviding by samples
		color_scale(c, c, 1.0/ (COORD_T) s->gfx->antialias_samples);
	}

	// Add ambient
	color_add(c, c, &s->ambient);
}
