#include "ik/vector3.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------------- */
void
vector3_set_zero(struct vector3_t* v)
{
    memset(v, 0, sizeof *v);
}

/* ------------------------------------------------------------------------- */
ik_real
vector3_length_squared(const struct vector3_t* v)
{
    return vector3_dot(v, v);
}

/* ------------------------------------------------------------------------- */
ik_real
vector3_length(const struct vector3_t* v)
{
    return sqrt(vector3_length_squared(v));
}

/* ------------------------------------------------------------------------- */
void
vector3_normalise(struct vector3_t* v)
{
    ik_real length = vector3_length(v);
    v->x /= length;
    v->y /= length;
    v->z /= length;
}

/* ------------------------------------------------------------------------- */
ik_real
vector3_dot(const struct vector3_t* v1, const struct vector3_t* v2)
{
    return v1->x * v2->x +
           v1->y * v2->y +
           v1->z * v2->z;
}
