#ifndef QUATERNION_H
#define QUATERNION_H

#include "ik/config.h"
#include "ik/vec3.h"

C_BEGIN

typedef union quat_t
{
    struct {
        ikreal_t x;
        ikreal_t y;
        ikreal_t z;
        ikreal_t w;
    };
    struct {
        vec3_t v;
    };
    ikreal_t f[4];
} quat_t;

/*!
 * @brief Sets the quaternion to its identity rotation.
 */
IK_PRIVATE_API void
quat_set_identity(ikreal_t q[4]);

/*!
 * @brief Copies elements from another quaternion.
 */
IK_PRIVATE_API void
quat_set(ikreal_t q[4], const ikreal_t src[4]);

/*!
 * @brief Adds the elements from one quaternion to another. Required for
 * averaging multiple quaternions.
 */
IK_PRIVATE_API void
quat_add_quat(ikreal_t q1[4], const ikreal_t q2[4]);

/*!
 * @brief Calculates the magnitude of a quaternion.
 */
IK_PRIVATE_API ikreal_t
quat_mag(const ikreal_t q[4]);

/*!
 * @brief Inverts the sign of the vector part of the quaternion (conjugation).
 */
IK_PRIVATE_API void
quat_conj(ikreal_t q[4]);

/*!
 * @brief Inverts the sign of all elements (NOT conjugation).
 */
IK_PRIVATE_API void
quat_invert_sign(ikreal_t q[4]);

/*!
 * @brief Normalises the quaternion.
 */
IK_PRIVATE_API void
quat_normalize(ikreal_t q[4]);

/*!
 * @brief Multiplies two quaternions together.
 */
IK_PRIVATE_API void
quat_mul_quat(ikreal_t q1[4], const ikreal_t q2[4]);

/*!
 * @brief Multiplies each component by a constant.
 */
IK_PRIVATE_API void
quat_mul_scalar(ikreal_t q[4], ikreal_t scalar);

/*!
 * @brief Divides each component by a constant. If the constant is 0 then the
 * result will be a unit quaternion.
 */
IK_PRIVATE_API void
quat_div_scalar(ikreal_t q[4], ikreal_t scalar);

/*!
 * @brief Calculates the scalar product of two quaternions.
 */
IK_PRIVATE_API ikreal_t
quat_dot(const ikreal_t q1[4], const ikreal_t q2[4]);

/*!
 * @brief Returns 0 if the two quaternions are "close", i.e. if -q has a
 * similar rotation as q.
 */
IK_PRIVATE_API void
quat_normalize_sign(ikreal_t q1[4]);

/*!
 * @brief Calculates the angle between two vectors. If the angle is 0 or 180,
 * the delta rotation is set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector.
 * @param[in] v2 The second vector.
 */
IK_PRIVATE_API void
quat_angle_unnormalized(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3]);

/*!
 * @brief Calculates the angle between two normalizedvectors. If the angle is
 * 0 or 180, the delta rotation is set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector (must be nomralized).
 * @param[in] v2 The second vector (must be normalized).
 */
IK_PRIVATE_API void
quat_angle(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3]);

C_END

#endif /* QUATERNION_H */
