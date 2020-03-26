#ifndef CTRANS_H
#define CTRANS_H

#include "vector.h"

// Used to create a sort of "virtual" coordinate system where up is defined by a given vector
// Implemented from https://www.scratchapixel.com/lessons/3d-basic-rendering/global-illumination-path-tracing/global-illumination-path-tracing-practical-implementation
// These methods assume y is up

typedef struct {
	vector_t *up;

	// These are calculated from the given up vector
	vector_t ax;
	vector_t az;
} csystem_t;

// Inits the given csystem_t with a given up vector.
// Will overwrite everything in cs
void csystem_init(csystem_t *cs, vector_t *up);

// Translates the inside coordinates to real and saves them in real.
// Will overwrite real
void csystem_calc_real(csystem_t *cs, vector_t *inside, vector_t *real);

// Writes the inside coordinates of a random direction in the upper hemisphere, and saves in out
// One must provide random numbers r1 and r2 between 0 and 1
void csystem_hemisphere_random(csystem_t *cs, COORD_T r1, COORD_T r2, vector_t *out);

#endif
