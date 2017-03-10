#include "ik/vec3.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------------- */
void
vec3_set_zero(struct vec3_t* v)
{
    memset(v, 0, sizeof *v);
}

/* ------------------------------------------------------------------------- */
void
vec3_add_vec3(struct vec3_t* v1, const struct vec3_t* v2)
{
    v1->x += v2->x;
    v1->y += v2->y;
    v1->z += v2->z;
}

/* ------------------------------------------------------------------------- */
void
vec3_sub_vec3(struct vec3_t* v1, const struct vec3_t* v2)
{
    v1->x -= v2->x;
    v1->y -= v2->y;
    v1->z -= v2->z;
}

/* ------------------------------------------------------------------------- */
void
vec3_mul_scalar(struct vec3_t* v, ik_real scalar)
{
    v->x *= scalar;
    v->y *= scalar;
    v->z *= scalar;
}

/* ------------------------------------------------------------------------- */
void
vec3_divide_scalar(struct vec3_t* v, ik_real scalar)
{
    v->x /= scalar;
    v->y /= scalar;
    v->z /= scalar;
}

/* ------------------------------------------------------------------------- */
ik_real
vec3_length_squared(const struct vec3_t* v)
{
    return vec3_dot(v, v);
}

/* ------------------------------------------------------------------------- */
ik_real
vec3_length(const struct vec3_t* v)
{
    return sqrt(vec3_length_squared(v));
}

/* ------------------------------------------------------------------------- */
void
vec3_normalise(struct vec3_t* v)
{
    ik_real length = vec3_length(v);
    v->x /= length;
    v->y /= length;
    v->z /= length;
}

/* ------------------------------------------------------------------------- */
ik_real
vec3_dot(const struct vec3_t* v1, const struct vec3_t* v2)
{
    return v1->x * v2->x +
           v1->y * v2->y +
           v1->z * v2->z;
}
