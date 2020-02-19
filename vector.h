#ifndef VECTOR_H
#define VECTOR_H
	
#define COORD_T float
#define ZERO_APROX 1e-6

typedef struct {
	COORD_T x;
	COORD_T y;
	COORD_T z;
} vector_t;

// Set vector to specified coordinates
// If p is NULL it is created
vector_t *vector_set(vector_t *p, COORD_T x, COORD_T y, COORD_T z);

// Copy vector src. If dest is NULL a vector is allocated
vector_t *vector_copy(vector_t *dest, vector_t *src);

// Calculate the length of v
COORD_T vector_len(vector_t *v);

COORD_T vector_dot(vector_t *a, vector_t *b);

// Basic math functions
vector_t *vector_add(vector_t *dest, vector_t *a, vector_t *b);
vector_t *vector_sub(vector_t *dest, vector_t *a, vector_t *b);
vector_t *vector_mult(vector_t *dest, vector_t *a, vector_t *b);

vector_t *vector_cross(vector_t *dest, vector_t *a, vector_t *b);

vector_t *vector_scale(vector_t *dest, vector_t *a, COORD_T b);
vector_t *vector_scale_inv(vector_t *dest, vector_t *a, COORD_T b);

void vector_print(vector_t *v);
#endif
