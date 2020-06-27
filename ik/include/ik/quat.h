#ifndef QUATERNION_H
#define QUATERNION_H

#include "ik/config.h"
#include "ik/vec3.h"

C_BEGIN

union ik_quat
{
    struct {
        ikreal x;
        ikreal y;
        ikreal z;
        ikreal w;
    } q;
    struct {
        union ik_vec3 v;
        ikreal w;
    } v;
    ikreal f[4];
};

/*!
 * @brief Copies x, y, z, w components from another quaternion.
 * @param[out] q Destination quaternion.
 * @param[in] src Source quaternion to copy from.
 */
IK_PUBLIC_API void
ik_quat_copy(ikreal q[4], const ikreal src[4]);

/*!
 * @brief Sets the x, y, z, w components of the specified quaternion.
 * @param[out] q Destination quaternion.
 * @param[in] src Source quaternion to copy from.
 */
IK_PUBLIC_API void
ik_quat_set(ikreal q[4], ikreal x, ikreal y, ikreal z, ikreal w);

/*!
 * @brief Sets the quaternion to its identity rotation.
 */
IK_PUBLIC_API void
ik_quat_set_identity(ikreal q[4]);

IK_PUBLIC_API void
ik_quat_set_axis_angle(ikreal q[4], ikreal x, ikreal y, ikreal z, ikreal a);

/*!
 * @brief Adds the elements from one quaternion to another. Required for
 * averaging multiple quaternions.
 */
IK_PUBLIC_API void
ik_quat_add_quat(ikreal q1[4], const ikreal q2[4]);

IK_PUBLIC_API void
ik_quat_sub_quat(ikreal q1[4], const ikreal q2[4]);

/*!
 * @brief Calculates the magnitude of a quaternion.
 */
IK_PUBLIC_API ikreal
ik_quat_mag(const ikreal q[4]);

/*!
 * @brief Calculates the conjugate, which is done by negating the vector
 * portion of the quaternion: q(w,-xyz)
 */
IK_PUBLIC_API void
ik_quat_conj(ikreal q[4]);

/*!
 * @brief Negates the sign of all components (this is NOT the conjugate):
 * q(-wxyz)
 */
IK_PUBLIC_API void
ik_quat_negate(ikreal q[4]);

/*!
 * @brief Calculates the quaternion's inverse, which, if ||q|| = 1, is
 * identical to the quaternion's conjugate. This function is for inverting
 * quaternions that don't have unit length.
 */
IK_PUBLIC_API void
ik_quat_invert(ikreal q[4]);

/*!
 * @brief Normalizes the quaternion so ||q|| = 1
 */
IK_PUBLIC_API void
ik_quat_normalize(ikreal q[4]);

/*!
 * @brief Multiplies two quaternions q1*q2 and stores the result into q1.
 * This operation is not commutative. The resulting quaternion is
 * normalized. See mul_no_normalize() for a version of this function that
 * doesn't normalize the result.
 */
IK_PUBLIC_API void
ik_quat_mul_quat(ikreal q1[4], const ikreal q2[4]);

/*!
 * @brief Multiplies q2*q1 and stores the result into q1.
 * This operation is not commutative. The resulting quaternion is
 * normalized. See nmul_no_normalize() for a version of this function that
 * doesn't normalize the result.
 */
IK_PUBLIC_API void
ik_quat_nmul_quat(ikreal q1[4], const ikreal q2[4]);

/*!
 * @brief Multiplies the conjugate of the specified quaternion, combining
 * the rotations. This operation is not commutative. The resulting quaternion is
 * normalized. See nmul_no_normalize() for a version of this function that
 * doesn't normalize the result.
 */
IK_PUBLIC_API void
ik_quat_mul_quat_conj(ikreal q1[4], const ikreal q2[4]);

/*!
 * @brief Multiplies q1*q2 and stores the result in q1. The result is not
 * normalized. This operation is not commutative.
 */
IK_PUBLIC_API void
ik_quat_mul_quat_no_normalize(ikreal q1[4], const ikreal q2[4]);

/*!
 * @brief Multiplies q2*q1 and stores the result in q1. The result is not
 * normalized. This operation is not commutative.
 */
IK_PUBLIC_API void
ik_quat_nmul_quat_no_normalize(ikreal q1[4], const ikreal q2[4]);

/*!
 * @brief Multiplies q1*q2 and stores the result in q1. The result is not
 * normalized. This operation is not commutative.
 */
IK_PUBLIC_API void
ik_quat_mul_quat_no_normalize(ikreal q1[4], const ikreal q2[4]);

/*!
 * @brief Multiplies the conjugate of the specified quaternion, combining
 * the rotations. This operation is not commutative. The resulting
 * quaternion is not normalized. This operation is not commutative.
 */
IK_PUBLIC_API void
ik_quat_mul_quat_conj_no_normalize(ikreal q1[4], const ikreal q2[4]);

/*!
 * @brief Multiplies each component by a scalar value.
 */
IK_PUBLIC_API void
ik_quat_mul_scalar(ikreal q[4], ikreal scalar);

/*!
 * @brief Divides each component by a scalar value. If the constant is 0 then the
 * result will be a unit quaternion.
 */
IK_PUBLIC_API void
ik_quat_div_scalar(ikreal q[4], ikreal scalar);

/*!
 * @brief Calculates the scalar product of two quaternions.
 */
IK_PUBLIC_API ikreal
ik_quat_dot(const ikreal q1[4], const ikreal q2[4]);

/*!
 * @brief Ensures the "sign" of the quaternion is not negative.
 * The main purpose of this is to assist in averaging the rotation of
 * multiple quaternions. Because -q is the same rotation as q, simply
 * adding quaternions together won't always cut it; you can only average
 * quaternions who's signs are all positive (or negative).
 */
IK_PUBLIC_API void
ik_quat_ensure_positive_sign(ikreal q1[4]);

/*!
 * @brief Calculates the angle between two vectors and assumes those vectors
 * are **NOT** normalized. If the angle is 0 or 180, the delta rotation is
 * set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector.
 * @param[in] v2 The second vector.
 */
IK_PUBLIC_API void
ik_quat_angle_between(ikreal q[4], const ikreal v1[3], const ikreal v2[3]);

/*!
 * @brief Calculates the absolute angle of the vector. By convention bones are
 * aligned to the Z axis, so this will return the angle between the specified
 * vector and the vector [0, 0, 1].
 */
IK_PUBLIC_API void
ik_quat_angle_of(ikreal q[4], const ikreal v[3]);

/*!
 * @brief Calculates the angle between two **normalized** vectors. If the
 * angle is 0 or 180, the delta rotation is set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector (must be nomralized).
 * @param[in] v2 The second vector (must be normalized).
 */
IK_PUBLIC_API void
ik_quat_angle_between_no_normalize(ikreal q[4], const ikreal v1[3], const ikreal v2[3]);

/*!
 * @brief Multiplies q by the rotation between two vectors. Equivalent to calling
 * ik_quat_angle_between() followed by ik_quat_mul_quat().
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector.
 * @param[in] v2 The second vector.
 */
IK_PUBLIC_API void
ik_quat_mul_angle_between(ikreal q[4], const ikreal v1[3], const ikreal v2[3]);

/*!
 * @brief Multiplies q by the absolute angle of the specified vector. By
 * convention bones are aligned to the Z axis, so this will multiply q by the
 * angle  between the specified vector and the vector [0, 0, 1]. This is Equivalent
 * to calling ik_quat_angle_of() followed by ik_quat_mul_quat().
 */
IK_PUBLIC_API void
ik_quat_mul_angle_of(ikreal q[4], const ikreal v[3]);

IK_PUBLIC_API int
ik_quat_prints(char* buf, const ikreal q[4]);

C_END

#endif /* QUATERNION_H */
