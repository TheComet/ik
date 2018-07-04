#ifndef QUATERNION_H
#define QUATERNION_H

#include "ik/config.h"
#include "ik/vec3.h"

C_BEGIN

typedef union ik_quat_t
{
    struct {
        ikreal_t x;
        ikreal_t y;
        ikreal_t z;
        ikreal_t w;
    };
    struct {
        ik_vec3_t v;
        ikreal_t _w;
    };
    ikreal_t f[4];
} ik_quat_t;

IK_INTERFACE(quat_interface)
{
    ik_quat_t
    (*quat)(ikreal_t x, ikreal_t y, ikreal_t z, ikreal_t w);

    /*!
     * @brief Sets the quaternion to its identity rotation.
     */
    void
    (*set_identity)(ikreal_t q[4]);

    /*!
     * @brief Copies elements from another quaternion.
     */
    void
    (*set)(ikreal_t q[4], const ikreal_t src[4]);

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
     * @brief Inverts the sign of the vector part of the quaternion (conjugation).
     */
    void
    (*conj)(ikreal_t q[4]);

    /*!
     * @brief Inverts the sign of all elements (NOT conjugation).
     */
    void
    (*invert_sign)(ikreal_t q[4]);

    /*!
     * @brief Normalises the quaternion.
     */
    void
    (*normalize)(ikreal_t q[4]);

    /*!
     * @brief Multiplies two quaternions together.
     */
    void
    (*mul_quat)(ikreal_t q1[4], const ikreal_t q2[4]);

    /*!
     * @brief Multiplies each component by a constant.
     */
    void
    (*mul_scalar)(ikreal_t q[4], ikreal_t scalar);

    /*!
     * @brief Divides each component by a constant. If the constant is 0 then the
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
     * @brief Returns 0 if the two quaternions are "close", i.e. if -q has a
     * similar rotation as q.
     */
    void
    (*normalize_sign)(ikreal_t q1[4]);

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
    (*angle_normalized_vectors)(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3]);
};

C_END

#endif /* QUATERNION_H */
