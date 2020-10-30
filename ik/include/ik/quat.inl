#pragma once

#include "ik/quat.h"
#include "ik/vec3.inl"

C_BEGIN

/*!
 * @brief Copies x, y, z, w components from another quaternion.
 * @param[out] q Destination quaternion.
 * @param[in] src Source quaternion to copy from.
 */
static inline void
ik_quat_copy(ikreal q[4], const ikreal src[4])
{
    q[0] = src[0];
    q[1] = src[1];
    q[2] = src[2];
    q[3] = src[3];
}

/*!
 * @brief Copies x, y, z, w components from another quaternion and conjugates
 * it.
 * @param[out] q Destination quaternion.
 * @param[in] src Source quaternion to copy from.
 */
static inline void
ik_quat_copy_conj(ikreal q[4], const ikreal src[4])
{
    q[0] = -src[0];
    q[1] = -src[1];
    q[2] = -src[2];
    q[3] = src[3];
}

/*!
 * @brief Sets the x, y, z, w components of the specified quaternion.
 * @param[out] q Destination quaternion.
 * @param[in] src Source quaternion to copy from.
 */
static inline void
ik_quat_set(ikreal q[4], ikreal x, ikreal y, ikreal z, ikreal w)
{
    q[0] = x;
    q[1] = y;
    q[2] = z;
    q[3] = w;
}

/*!
 * @brief Sets the quaternion to its identity rotation.
 */
static inline void
ik_quat_set_identity(ikreal q[4])
{
    q[0] = 0;
    q[1] = 0;
    q[2] = 0;
    q[3] = 1;
}

static inline void
ik_quat_set_axis_angle(ikreal q[4], ikreal x, ikreal y, ikreal z, ikreal a)
{
    ik_vec3_set(q, x, y, z);
    ik_vec3_normalize(q);
    ik_vec3_mul_scalar(q, sin(a * 0.5));
    q[3] = cos(a * 0.5);
}

/*!
 * @brief Adds the elements from one quaternion to another. Required for
 * averaging multiple quaternions.
 */
static inline void
ik_quat_add_quat(ikreal q1[4], const ikreal q2[4])
{
    q1[0] += q2[0];
    q1[1] += q2[1];
    q1[2] += q2[2];
    q1[3] += q2[3];
}

static inline void
ik_quat_sub_quat(ikreal q1[4], const ikreal q2[4])
{
    q1[0] -= q2[0];
    q1[1] -= q2[1];
    q1[2] -= q2[2];
    q1[3] -= q2[3];
}

/*!
 * @brief Negates the sign of all components (this is NOT the conjugate):
 * q(-wxyz)
 */
static inline void
ik_quat_negate(ikreal q[4])
{
    q[0] = -q[0];
    q[1] = -q[1];
    q[2] = -q[2];
    q[3] = -q[3];
}

/*!
 * @brief Multiplies each component by a scalar value.
 */
static inline void
ik_quat_mul_scalar(ikreal q[4], ikreal scalar)
{
    q[0] *= scalar;
    q[1] *= scalar;
    q[2] *= scalar;
    q[3] *= scalar;
}

/*!
 * @brief Divides each component by a scalar value. If the constant is 0 then the
 * result will be a unit quaternion.
 */
static inline void
ik_quat_div_scalar(ikreal q[4], ikreal scalar)
{
    if (scalar == 0.0)
        ik_quat_set_identity(q);
    else
    {
        ikreal rec = 1.0 / scalar;
        q[0] *= rec;
        q[1] *= rec;
        q[2] *= rec;
        q[3] *= rec;
    }
}

/*!
 * @brief Ensures the "sign" of the quaternion is not negative.
 * The main purpose of this is to assist in averaging the rotation of
 * multiple quaternions. Because -q is the same rotation as q, simply
 * adding quaternions together won't always cut it; you can only average
 * quaternions who's signs are all positive (or negative).
 */
static inline void
ik_quat_ensure_positive_sign(ikreal q[4])
{
    if (q[3] < 0.0)
        ik_quat_negate(q);
}

/*!
 * @brief Calculates the scalar product of two quaternions.
 */
static inline ikreal
ik_quat_dot(const ikreal q1[4], const ikreal q2[4])
{
    return q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];
}

/*!
 * @brief Calculates the magnitude of a quaternion.
 */
static inline ikreal
ik_quat_mag(const ikreal q[4])
{
    return sqrt(q[3]*q[3] + q[2]*q[2] + q[1]*q[1] + q[0]*q[0]);
}

/*!
 * @brief Calculates the conjugate, which is done by negating the vector
 * portion of the quaternion: q(w,-xyz)
 */
static inline void
ik_quat_conj(ikreal q[4])
{
    q[0] = -q[0];
    q[1] = -q[1];
    q[2] = -q[2];
}

/*!
 * @brief Calculates the quaternion's inverse, which, if ||q|| = 1, is
 * identical to the quaternion's conjugate. This function is for inverting
 * quaternions that don't have unit length.
 */
static inline void
ik_quat_invert(ikreal q[4])
{
    ikreal mag_squared = ik_quat_dot(q, q);
    ik_quat_conj(q);
    ik_quat_div_scalar(q, mag_squared);
}

/*!
 * @brief Normalizes the quaternion so ||q|| = 1
 */
static inline void
ik_quat_normalize(ikreal q[4])
{
    ikreal r_mag = 1.0 / ik_quat_mag(q);
    q[0] *= r_mag;
    q[1] *= r_mag;
    q[2] *= r_mag;
    q[3] *= r_mag;
}

/*! @brief q1.q2 -> q1 without normalization of the result. */
static inline void
ik_quat_mul_quat_nn(ikreal q[4], const ikreal q2[4])
{
    ikreal q1[4];
    ik_quat_copy(q1, q);

    q[3] =  q1[3]*q2[3] - q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2];
    q[0] =  q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
    q[1] =  q1[3]*q2[1] + q1[1]*q2[3] + q1[2]*q2[0] - q1[0]*q2[2];
    q[2] =  q1[3]*q2[2] + q1[2]*q2[3] + q1[0]*q2[1] - q1[1]*q2[0];
}

/*! @brief q1*q2 -> q2 without normalization of the result. */
static inline void
ik_quat_rmul_quat_nn(const ikreal q1[4], ikreal q[4])
{
    ikreal q2[4];
    ik_quat_copy(q2, q);

    q[3] =  q1[3]*q2[3] - q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2];
    q[0] =  q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
    q[1] =  q1[3]*q2[1] + q1[1]*q2[3] + q1[2]*q2[0] - q1[0]*q2[2];
    q[2] =  q1[3]*q2[2] + q1[2]*q2[3] + q1[0]*q2[1] - q1[1]*q2[0];
}

/*! @brief q1.q2* -> q1 without normalization of the result.  */
static inline void
ik_quat_mul_quat_conj_nn(ikreal q[4], const ikreal q2[4])
{
    ikreal q1[4];
    ik_quat_copy(q1, q);

    q[3] =  q1[3]*q2[3] + q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2];
    q[0] = -q1[3]*q2[0] + q1[0]*q2[3] - q1[1]*q2[2] + q1[2]*q2[1];
    q[1] = -q1[3]*q2[1] + q1[1]*q2[3] - q1[2]*q2[0] + q1[0]*q2[2];
    q[2] = -q1[3]*q2[2] + q1[2]*q2[3] - q1[0]*q2[1] + q1[1]*q2[0];
}

/*! @brief q1.q2* -> q2 without normalization of the result.  */
static inline void
ik_quat_rmul_quat_conj_nn(const ikreal q1[4], ikreal q[4])
{
    ikreal q2[4];
    ik_quat_copy(q2, q);

    q[3] =  q1[3]*q2[3] + q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2];
    q[0] = -q1[3]*q2[0] + q1[0]*q2[3] - q1[1]*q2[2] + q1[2]*q2[1];
    q[1] = -q1[3]*q2[1] + q1[1]*q2[3] - q1[2]*q2[0] + q1[0]*q2[2];
    q[2] = -q1[3]*q2[2] + q1[2]*q2[3] - q1[0]*q2[1] + q1[1]*q2[0];
}

/*! @brief q1*.q2 -> q1 without normalization of the result.  */
static inline void
ik_quat_conj_mul_quat_nn(ikreal q[4], const ikreal q2[4])
{
    ikreal q1[4];
    ik_quat_copy(q1, q);

    q[3] =  q1[3]*q2[3] + q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2];
    q[0] =  q1[3]*q2[0] - q1[0]*q2[3] - q1[1]*q2[2] + q1[2]*q2[1];
    q[1] =  q1[3]*q2[1] - q1[1]*q2[3] - q1[2]*q2[0] + q1[0]*q2[2];
    q[2] =  q1[3]*q2[2] - q1[2]*q2[3] - q1[0]*q2[1] + q1[1]*q2[0];
}

/*! @brief q1*.q2 -> q2 without normalization of the result.  */
static inline void
ik_quat_conj_rmul_quat_nn(const ikreal q1[4], ikreal q[4])
{
    ikreal q2[4];
    ik_quat_copy(q2, q);

    q[3] =  q1[3]*q2[3] + q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2];
    q[0] =  q1[3]*q2[0] - q1[0]*q2[3] - q1[1]*q2[2] + q1[2]*q2[1];
    q[1] =  q1[3]*q2[1] - q1[1]*q2[3] - q1[2]*q2[0] + q1[0]*q2[2];
    q[2] =  q1[3]*q2[2] - q1[2]*q2[3] - q1[0]*q2[1] + q1[1]*q2[0];
}

#define OPL_NORM(name)                                 \
    static inline void                                 \
    ik_##name(ikreal q1[4], const ikreal q2[4]) {      \
        ik_##name##_nn(q1, q2);                        \
        ik_quat_normalize(q1);                         \
    }
#define OPR_NORM(name)                                 \
    static inline void                                 \
    ik_##name(const ikreal q1[4], ikreal q2[4]) {      \
        ik_##name##_nn(q1, q2);                        \
        ik_quat_normalize(q2);                         \
    }

/*! @brief q1.q2 -> q1 with normalization of the result. */
OPL_NORM(quat_mul_quat)

/*! @brief q1*q2 -> q2 with normalization of the result. */
OPR_NORM(quat_rmul_quat)

/*! @brief q1.q2* -> q1 with normalization of the result.  */
OPL_NORM(quat_mul_quat_conj)

/*! @brief q1.q2* -> q2 with normalization of the result.  */
OPR_NORM(quat_rmul_quat_conj)

/*! @brief q1*.q2 -> q1 with normalization of the result.  */
OPL_NORM(quat_conj_mul_quat)

/*! @brief q1*.q2 -> q2 with normalization of the result.  */
OPR_NORM(quat_conj_rmul_quat)

#undef OPL_NORM
#undef OPR_NORM

/*!
 * @brief Calculates the angle between two vectors and assumes those vectors
 * are **NOT** normalized. If the angle is 0 or 180, the delta rotation is
 * set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector.
 * @param[in] v2 The second vector.
 */
static inline void
ik_quat_angle_between(ikreal q[4], const ikreal v1[3], const ikreal v2[3])
{
    ikreal cos_a, sin_a, angle, denominator;

    denominator = 1.0 / ik_vec3_length(v1) / ik_vec3_length(v2);
    cos_a = ik_vec3_dot(v1, v2) * denominator;
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        ik_vec3_copy(q, v1);
        ik_vec3_cross(q, v2);
        ik_vec3_normalize(q);

        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(cos_a);
        cos_a = cos(angle * 0.5);
        sin_a = sin(angle * 0.5);
        ik_vec3_mul_scalar(q, sin_a);
        q[3] = cos_a; /* w component */
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        ik_quat_set_identity(q);
    }
}

/*!
 * @brief Calculates the angle between two **normalized** vectors. If the
 * angle is 0 or 180, the delta rotation is set to identity.
 * @param[out] q A contiguous array of 4 ik_floats representing a quaternion.
 * The result is written to this. Any previous data is overwritten.
 * @param[in] v1 The first vector (must be nomralized).
 * @param[in] v2 The second vector (must be normalized).
 */
static inline void
ik_quat_angle_between_nn(ikreal q[4], const ikreal v1[3], const ikreal v2[3])
{
    ikreal cos_a, sin_a, angle;

    cos_a = ik_vec3_dot(v1, v2);
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        ik_vec3_copy(q, v1);
        ik_vec3_cross(q, v2);
        ik_vec3_normalize(q);

        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(cos_a);
        cos_a = cos(angle * 0.5);
        sin_a = sin(angle * 0.5);
        ik_vec3_mul_scalar(q, sin_a);
        q[3] = cos_a; /* w component */
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        ik_quat_set_identity(q);
    }
}

/*!
 * @brief Calculates the absolute angle of the vector. By convention bones are
 * aligned to the Z axis, so this will return the angle between the specified
 * vector and the vector [0, 0, 1].
 */
static inline void
ik_quat_angle_of(ikreal q[4], const ikreal v[3])
{
    ikreal cos_a, sin_a, angle;

    cos_a = v[2] / ik_vec3_length(v);
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(cos_a);
        sin_a = sin(angle * 0.5);

        /* cross product of [0, 0, 1] with v, store result into q */
        q[0] = -v[1];
        q[1] = v[0];
        q[2] = 0;
        /* normalize/weight vector part of quaternion */
        ik_vec3_normalize(q);
        q[0] *= sin_a;
        q[1] *= sin_a;
        /* w component */
        q[3] = cos(angle * 0.5);
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        ik_quat_set_identity(q);
    }
}

static inline void
ik_quat_angle_of_nn(ikreal q[4], const ikreal v[3])
{
    ikreal sin_a, angle;

    if (v[2] >= -1.0 && v[2] <= 1.0)
    {
        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(v[2]);
        sin_a = sin(angle * 0.5);

        /* cross product of [0, 0, 1] with v, store result into q */
        q[0] = -v[1];
        q[1] = v[0];
        q[2] = 0;
        /* normalize/weight vector part of quaternion */
        if (!ik_vec3_normalize(q))
            ik_vec3_set(q, 1, 0, 0);  /* we are forced to choose an axis */
        q[0] *= sin_a;
        q[1] *= sin_a;
        /* w component */
        q[3] = cos(angle * 0.5);
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        ik_quat_set_identity(q);
    }
}

C_END
