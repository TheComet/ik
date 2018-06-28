#ifndef VEC3_H
#define VEC3_H

#include "ik/config.h"

C_HEADER_BEGIN

typedef union vec3_t
{
    struct {
        ik_real x;
        ik_real y;
        ik_real z;
    };
    ik_real f[3];
} vec3_t;

IK_PRIVATE_API void
vec3_copy(ik_real v[3], const ik_real src[3]);

IK_PRIVATE_API void
vec3_set_zero(ik_real v[3]);

IK_PRIVATE_API void
vec3_add_vec3(ik_real v1[3], const ik_real v2[3]);

IK_PRIVATE_API void
vec3_sub_vec3(ik_real v1[3], const ik_real v2[3]);

IK_PRIVATE_API void
vec3_mul_scalar(ik_real v1[3], ik_real scalar);

IK_PRIVATE_API void
vec3_div_scalar(ik_real v[3], ik_real scalar);

IK_PRIVATE_API ik_real
vec3_length_squared(const ik_real v[3]);

IK_PRIVATE_API ik_real
vec3_length(const ik_real v[3]);

IK_PRIVATE_API void
vec3_normalize(ik_real v[3]);

IK_PRIVATE_API ik_real
vec3_dot(const ik_real v1[3], const ik_real v2[3]);

IK_PRIVATE_API void
vec3_cross(ik_real v1[3], const ik_real v2[3]);

/*!
 * @brief Calculates the angle between two vectors. If the angle is 0 or 180,
 * the delta rotation is set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector.
 * @param[in] v2 The second vector.
 */
IK_PRIVATE_API void
vec3_angle(ik_real q[4], const ik_real v1[3], const ik_real v2[3]);

/*!
 * @brief Calculates the angle between two normalized vectors. If the angle is
 * 0 or 180, the delta rotation is set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector (must be nomralized).
 * @param[in] v2 The second vector (must be normalized).
 */
IK_PRIVATE_API void
vec3_angle_normalized(ik_real q[4], const ik_real v1[3], const ik_real v2[3]);

C_HEADER_END

#endif /* VEC3_H */
