#ifndef VEC3_H
#define VEC3_H

#include "ik/config.h"

C_HEADER_BEGIN

struct vec3_t
{
    ik_real x;
    ik_real y;
    ik_real z;
};

IK_PUBLIC_API void
vec3_set_zero(struct vec3_t* v);

IK_PUBLIC_API void
vec3_add_vec3(struct vec3_t* v1, const struct vec3_t* v2);

IK_PUBLIC_API void
vec3_sub_vec3(struct vec3_t* v1, const struct vec3_t* v2);

IK_PUBLIC_API void
vec3_mul_scalar(struct vec3_t* v1, ik_real scalar);

IK_PUBLIC_API void
vec3_divide_scalar(struct vec3_t* v, ik_real scalar);

IK_PUBLIC_API ik_real
vec3_length_squared(const struct vec3_t* v);

IK_PUBLIC_API ik_real
vec3_length(const struct vec3_t* v);

IK_PUBLIC_API void
vec3_normalise(struct vec3_t* v);

IK_PUBLIC_API ik_real
vec3_dot(const struct vec3_t* v1, const struct vec3_t* v2);

C_HEADER_END

#endif /* VEC3_H */
