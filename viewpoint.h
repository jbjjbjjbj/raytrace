#ifndef VIEWPOINT_H
#define VIEWPOINT_H

#include "vector.h"

// 100 degrees
#define FOV 1.74533

typedef struct {
	// Viewpoint dimensions
	unsigned int width;
	unsigned int height;

	// Position and orientation
	vector_t position, target;

	// Viewpoint shift vectors
	// Calculated at initialisation
	vector_t qx, qy;
	// Left button center pixel location
	vector_t blc;
} viewpoint_t;

// Todo handle initial setup
void viewpoint_init(viewpoint_t *view);

void viewpoint_ray(viewpoint_t *view, vector_t *r, unsigned x, unsigned y);
#endif
