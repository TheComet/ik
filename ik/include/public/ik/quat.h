#ifndef QUATERNION_H
#define QUATERNION_H

#include "ik/config.h"
#include "ik/vec3.h"

C_BEGIN

union ik_quat_t
{
    struct {
        ikreal_t x;
        ikreal_t y;
        ikreal_t z;
        ikreal_t w;
    } q;
    struct {
        union ik_vec3_t v;
        ikreal_t _w;
    } v;
    ikreal_t f[4];
};

#if defined(IK_BUILDING)

/*!
 * @brief Copies x, y, z, w components from another quaternion.
 * @param[out] q Destination quaternion.
 * @param[in] src Source quaternion to copy from.
 */
IK_PRIVATE_API void
ik_quat_copy(ikreal_t q[4], const ikreal_t src[4]);

/*!
 * @brief Sets the x, y, z, w components of the specified quaternion.
 * @param[out] q Destination quaternion.
 * @param[in] src Source quaternion to copy from.
 */
IK_PRIVATE_API void
ik_quat_set(ikreal_t q[4], ikreal_t x, ikreal_t y, ikreal_t z, ikreal_t w);

/*!
 * @brief Sets the quaternion to its identity rotation.
 */
IK_PRIVATE_API void
ik_quat_set_identity(ikreal_t q[4]);

IK_PRIVATE_API void
ik_quat_set_axis_angle(ikreal_t q[4], ikreal_t x, ikreal_t y, ikreal_t z, ikreal_t a);

/*!
 * @brief Adds the elements from one quaternion to another. Required for
 * averaging multiple quaternions.
 */
IK_PRIVATE_API void
ik_quat_add_quat(ikreal_t q1[4], const ikreal_t q2[4]);

/*!
 * @brief Calculates the magnitude of a quaternion.
 */
IK_PRIVATE_API ikreal_t
ik_quat_mag(const ikreal_t q[4]);

/*!
 * @brief Calculates the conjugate, which is done by negating the vector
 * portion of the quaternion: q(w,-xyz)
 */
IK_PRIVATE_API void
ik_quat_conj(ikreal_t q[4]);

/*!
 * @brief Negates the sign of all components (this is NOT the conjugate):
 * q(-wxyz)
 */
IK_PRIVATE_API void
ik_quat_negate(ikreal_t q[4]);

/*!
 * @brief Calculates the quaternion's inverse, which, if ||q|| = 1, is
 * identical to the quaternion's conjugate. This function is for inverting
 * quaternions that don't have unit length.
 */
IK_PRIVATE_API void
ik_quat_invert(ikreal_t q[4]);

/*!
 * @brief Normalizes the quaternion so ||q|| = 1
 */
IK_PRIVATE_API void
ik_quat_normalize(ikreal_t q[4]);

/*!
 * @brief Multiplies two quaternions together, combining the rotations.
 * This operation is not commutative. The resulting quaternion is
 * normalized. See mul_no_normalize() for a version of this function that
 * doesn't normalize the result.
 */
IK_PRIVATE_API void
ik_quat_mul_quat(ikreal_t q1[4], const ikreal_t q2[4]);

/*!
 * @brief Multiplies the conjugate of the specified quaternion, combining
 * the rotations. This operation is not commutative. The resulting quaternion is
 * normalized. See nmul_no_normalize() for a version of this function that
 * doesn't normalize the result.
 */
IK_PRIVATE_API void
ik_quat_nmul_quat(ikreal_t q1[4], const ikreal_t q2[4]);

/*!
 * @brief Multiplies two quaternions together, combining the rotations, and
 * does not normalize the result. This operation is not commutative.
 */
IK_PRIVATE_API void
ik_quat_mul_no_normalize(ikreal_t q1[4], const ikreal_t q2[4]);

/*!
 * @brief Multiplies the conjugate of the specified quaternion, combining
 * the rotations. This operation is not commutative. The resulting
 * quaternion is not normalized. This operation is not commutative.
 */
IK_PRIVATE_API void
ik_quat_nmul_no_normalize(ikreal_t q1[4], const ikreal_t q2[4]);

/*!
 * @brief Multiplies each component by a scalar value.
 */
IK_PRIVATE_API void
ik_quat_mul_scalar(ikreal_t q[4], ikreal_t scalar);

/*!
 * @brief Divides each component by a scalar value. If the constant is 0 then the
 * result will be a unit quaternion.
 */
IK_PRIVATE_API void
ik_quat_div_scalar(ikreal_t q[4], ikreal_t scalar);

/*!
 * @brief Calculates the scalar product of two quaternions.
 */
IK_PRIVATE_API ikreal_t
ik_quat_dot(const ikreal_t q1[4], const ikreal_t q2[4]);

/*!
 * @brief Ensures the "sign" of the quaternion is not negative.
 * The main purpose of this is to assist in averaging the rotation of
 * multiple quaternions. Because -q is the same rotation as q, simply
 * adding quaternions together won't always cut it; you can only average
 * quaternions who's signs are all positive (or negative).
 */
IK_PRIVATE_API void
ik_quat_ensure_positive_sign(ikreal_t q1[4]);

/*!
 * @brief Calculates the angle between two vectors and assumes those vectors
 * are **NOT** normalized. If the angle is 0 or 180, the delta rotation is
 * set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector.
 * @param[in] v2 The second vector.
 */
IK_PRIVATE_API void
ik_quat_angle(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3]);

/*!
 * @brief Calculates the angle between two **normalized** vectors. If the
 * angle is 0 or 180, the delta rotation is set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector (must be nomralized).
 * @param[in] v2 The second vector (must be normalized).
 */
IK_PRIVATE_API void
ik_quat_angle_no_normalize(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3]);

IK_PRIVATE_API int
ik_quat_print(char* buf, const ikreal_t q[4]);

#endif /* IK_BUILDING */

C_END

#endif /* QUATERNION_H */
