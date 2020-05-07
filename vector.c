#include "vector.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <assert.h>

static inline vector_t *vector_exists(vector_t *v)
{
	assert(v);

	return v;
}

// Overwrites stuff in p. If p is null a new vector is created
vector_t *vector_set(vector_t *p, COORD_T x, COORD_T y, COORD_T z)
{
	p = vector_exists(p);

	p->x = x;
	p->y = y;
	p->z = z;

	return p;
}

// Can also be used to create a vector by leaving v NULL
vector_t *vector_copy(vector_t *dest, vector_t *src)
{
	dest = vector_exists(dest);

	// If v exist copy it over
	if (src) {
		memcpy(dest, src, sizeof(vector_t));
	}

	return dest;
}

COORD_T vector_len(vector_t *v)
{
	return sqrt( v->x * v->x + v->y * v->y + v->z * v->z );
}

void vector_norm(vector_t *v)
{
    COORD_T len = vector_len(v);
    if (len == 0) {
        vector_set(v, 0, 0, 0);
        return;
    }

    vector_scale_inv(v, v, len);
}

vector_t *vector_add(vector_t *dest, vector_t *a, vector_t *b)
{
	dest = vector_exists(dest);

	dest->x = a->x + b->x;
	dest->y = a->y + b->y;
	dest->z = a->z + b->z;

	return dest;
}

vector_t *vector_sub(vector_t *dest, vector_t *a, vector_t *b)
{
	dest = vector_exists(dest);

	dest->x = a->x - b->x;
	dest->y = a->y - b->y;
	dest->z = a->z - b->z;

	return dest;
}

vector_t *vector_mult(vector_t *dest, vector_t *a, vector_t *b)
{
	dest = vector_exists(dest);

	dest->x = a->x * b->x;
	dest->y = a->y * b->y;
	dest->z = a->z * b->z;

	return dest;
}

vector_t *vector_scale(vector_t *dest, vector_t *a, COORD_T b)
{
	dest = vector_exists(dest);

	dest->x = a->x * b;
	dest->y = a->y * b;
	dest->z = a->z * b;

	return dest;
}

vector_t *vector_scale_inv(vector_t *dest, vector_t *a, COORD_T b)
{
	dest = vector_exists(dest);

	dest->x = a->x / b;
	dest->y = a->y / b;
	dest->z = a->z / b;

	return dest;
}

COORD_T vector_dot(vector_t *a, vector_t *b)
{
	return a->x * b->x + a->y * b->y + a->z * b->z;
}

vector_t *vector_cross(vector_t *dest, vector_t *a, vector_t *b)
{
	dest = vector_exists(dest);

	dest->x = a->y * b->z - a->z * b->y;
	dest->y = a->z * b->x - a->x * b->z;
	dest->z = a->x * b->y - a->y * b->x;

	return dest;
}

void vector_print(vector_t *v)
{
	printf("[ %f, %f, %f ]\n", v->x, v->y, v->z);
}
