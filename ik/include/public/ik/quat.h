#ifndef QUATERNION_H
#define QUATERNION_H

#include "ik/config.h"
#include "ik/vec3.h"

C_BEGIN

struct ik_quat_t
{
    union
    {
        struct {
            ikreal_t x;
            ikreal_t y;
            ikreal_t z;
            ikreal_t w;
        };
        struct {
            struct ik_vec3_t v;
            ikreal_t _w;
        };
        ikreal_t f[4];
    };
};

IK_INTERFACE(quat_interface)
{
    /*!
     * @brief Convenience function for initializing a quaternion.
     */
    struct ik_quat_t
    (*quat)(ikreal_t x, ikreal_t y, ikreal_t z, ikreal_t w);

    /*!
     * @brief Sets the quaternion to its identity rotation.
     */
    void
    (*set_identity)(ikreal_t q[4]);

    /*!
     * @brief Copies x, y, z, w components from another quaternion.
     * @param[out] q Destination quaternion.
     * @param[in] src Source quaternion to copy from.
     */
    void
    (*set)(ikreal_t q[4], const ikreal_t src[4]);

    void
    (*set_axis_angle)(ikreal_t q[4], const ikreal_t v[3], ikreal_t a);

    /*!
     * @brief Adds the elements from one quaternion to another. Required for
     * averaging multiple quaternions.
     */
    void
    (*add_quat)(ikreal_t q1[4], const ikreal_t q2[4]);

    /*!
     * @brief Calculates the magnitude of a quaternion.
     */
    ikreal_t
    (*mag)(const ikreal_t q[4]);

    /*!
     * @brief Calculates the conjugate, which is done by negating the vector
     * portion of the quaternion: q(w,-xyz)
     */
    void
    (*conj)(ikreal_t q[4]);

    /*!
     * @brief Negates the sign of all components (this is NOT the conjugate):
     * q(-wxyz)
     */
    void
    (*negate)(ikreal_t q[4]);

    /*!
     * @brief Calculates the quaternion's inverse, which, if ||q|| = 1, is
     * identical to the quaternion's conjugate. This function is for inverting
     * quaternions that don't have unit length.
     */
    void
    (*invert)(ikreal_t q[4]);

    /*!
     * @brief Normalizes the quaternion so ||q|| = 1
     */
    void
    (*normalize)(ikreal_t q[4]);

    /*!
     * @brief Multiplies two quaternions together, combining the rotations.
     * This operation is not commutative. The resulting quaternion is
     * normalized. See mul_no_normalize() for a version of this function that
     * doesn't normalize the result.
     */
    void
    (*mul_quat)(ikreal_t q1[4], const ikreal_t q2[4]);

    /*!
     * @brief Multiplies two quaternions together, combining the rotations, and
     * does not normalize the result. This operation is not commutative.
     */
    void
    (*mul_no_normalize)(ikreal_t q1[4], const ikreal_t q2[4]);

    /*!
     * @brief Multiplies each component by a scalar value.
     */
    void
    (*mul_scalar)(ikreal_t q[4], ikreal_t scalar);

    /*!
     * @brief Divides each component by a scalar value. If the constant is 0 then the
     * result will be a unit quaternion.
     */
    void
    (*div_scalar)(ikreal_t q[4], ikreal_t scalar);

    /*!
     * @brief Calculates the scalar product of two quaternions.
     */
    ikreal_t
    (*dot)(const ikreal_t q1[4], const ikreal_t q2[4]);

    /*!
     * @brief Ensures the "sign" of the quaternion is not negative.
     * The main purpose of this is to assist in averaging the rotation of
     * multiple quaternions. Because -q is the same rotation as q, simply
     * adding quaternions together won't always cut it; you can only average
     * quaternions who's signs are all positive (or negative).
     */
    void
    (*ensure_positive_sign)(ikreal_t q1[4]);

    /*!
     * @brief Calculates the angle between two vectors and assumes those vectors
     * are **NOT** normalized. If the angle is 0 or 180, the delta rotation is
     * set to identity.
     * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
     * The result is written to this. Any previous data is overwritten.
     * @param[in] v1 The first vector.
     * @param[in] v2 The second vector.
     */
    void
    (*angle)(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3]);

    /*!
     * @brief Calculates the angle between two **normalized** vectors. If the
     * angle is 0 or 180, the delta rotation is set to identity.
     * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
     * The result is written to this. Any previous data is overwritten.
     * @param[in] v1 The first vector (must be nomralized).
     * @param[in] v2 The second vector (must be normalized).
     */
    void
    (*angle_no_normalize)(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3]);

    void
    (*print)(char* buf, const ikreal_t q[4]);
};

C_END

#endif /* QUATERNION_H */
