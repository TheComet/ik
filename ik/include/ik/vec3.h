#ifndef VEC3_H
#define VEC3_H

#include "ik/config.h"

C_BEGIN

typedef union vec3_t
{
    struct {
        ikreal_t x;
        ikreal_t y;
        ikreal_t z;
    };
    ikreal_t f[3];
} vec3_t;

IK_PRIVATE_API void
vec3_set(ikreal_t v[3], const ikreal_t src[3]);

IK_PRIVATE_API void
vec3_set_zero(ikreal_t v[3]);

IK_PRIVATE_API void
vec3_add_scalar(ikreal_t v1[3], ikreal_t scalar);

IK_PRIVATE_API void
vec3_add_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

IK_PRIVATE_API void
vec3_sub_scalar(ikreal_t v1[3], ikreal_t scalar);

IK_PRIVATE_API void
vec3_sub_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

IK_PRIVATE_API void
vec3_mul_scalar(ikreal_t v1[3], ikreal_t scalar);

IK_PRIVATE_API void
vec3_mul_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

IK_PRIVATE_API void
vec3_div_scalar(ikreal_t v[3], ikreal_t scalar);

IK_PRIVATE_API void
vec3_div_vec3(ikreal_t v[3], const ikreal_t v2[3]);

IK_PRIVATE_API ikreal_t
vec3_length_squared(const ikreal_t v[3]);

IK_PRIVATE_API ikreal_t
vec3_length(const ikreal_t v[3]);

IK_PRIVATE_API void
vec3_normalize(ikreal_t v[3]);

IK_PRIVATE_API ikreal_t
vec3_dot(const ikreal_t v1[3], const ikreal_t v2[3]);

IK_PRIVATE_API void
vec3_cross(ikreal_t v1[3], const ikreal_t v2[3]);

/*!
 * @brief Rotations a vector by the specified quaternion.
 */
IK_PRIVATE_API void
vec3_rotate(ikreal_t v[3], const ikreal_t q[4]);

C_END

#endif /* VEC3_H */
