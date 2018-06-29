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
vec3_copy(ikreal_t v[3], const ikreal_t src[3]);

IK_PRIVATE_API void
vec3_set_zero(ikreal_t v[3]);

IK_PRIVATE_API void
vec3_add_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

IK_PRIVATE_API void
vec3_sub_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

IK_PRIVATE_API void
vec3_mul_scalar(ikreal_t v1[3], ikreal_t scalar);

IK_PRIVATE_API void
vec3_div_scalar(ikreal_t v[3], ikreal_t scalar);

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
 * @brief Calculates the angle between two vectors. If the angle is 0 or 180,
 * the delta rotation is set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector.
 * @param[in] v2 The second vector.
 */
IK_PRIVATE_API void
vec3_angle(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3]);

/*!
 * @brief Calculates the angle between two normalized vectors. If the angle is
 * 0 or 180, the delta rotation is set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector (must be nomralized).
 * @param[in] v2 The second vector (must be normalized).
 */
IK_PRIVATE_API void
vec3_angle_normalized(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3]);

C_END

#endif /* VEC3_H */
