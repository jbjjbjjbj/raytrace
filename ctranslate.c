#include "ctranslate.h"
#include "vector.h"

#include <math.h>

// TODO remove dublicate pi definitions
#define PI 3.14159265359
#define DEBUG 0

void csystem_init(csystem_t *cs, vector_t *up)
{
	cs->up = up;

	// Create new coordinate system where N is up. To do this we need two more vectors for the other axises.
	// Create the 2. by setting x or y to 0
	if (fabs(up->x) > fabs(up->y)) {
		vector_set(&cs->ax, up->z, 0, -up->x);
	} else {
		vector_set(&cs->ax, 0, -up->z, up->y);
	}
	// Normalice
    vector_norm(&cs->ax);


	// Create the 3. axis by taking the cross of the other
	vector_cross(&cs->az, up, &cs->ax);
#if DEBUG == 1
    printf("up "); vector_print(up);
    printf("ax "); vector_print(&cs->ax);
    printf("az "); vector_print(&cs->az);
#endif
}

void csystem_calc_real(csystem_t *cs, vector_t *inside, vector_t *real)
{
    vector_t tmp;
	vector_set(&tmp, inside->x * cs->ax.x + inside->y * cs->up->x + inside->z * cs->az.x,
			         inside->x * cs->ax.y + inside->y * cs->up->y + inside->z * cs->az.y,
					 inside->x * cs->ax.z + inside->y * cs->up->z + inside->z * cs->az.z);

    vector_copy(real, &tmp);
#if DEBUG == 1
    printf("Haha "); vector_print(real);
#endif
}

void csystem_hemisphere_random(csystem_t *cs, COORD_T r1, COORD_T r2, vector_t *out)
{
	COORD_T sinTheta = sqrt(1 - r1 * r1);
	COORD_T phi = 2 * PI * r2;

	// Calculate the random direction vector
	vector_set(out, sinTheta * cos(phi), r1, sinTheta * sin(phi));
}

