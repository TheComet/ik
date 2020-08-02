#pragma once

#include "ik/vec3.h"
#include <math.h>

C_BEGIN

/*! Copy vector src into v */
static inline void
ik_vec3_copy(ikreal v[3], const ikreal src[3])
{
    v[0] = src[0];
    v[1] = src[1];
    v[2] = src[2];
}

/*! Set xyz components of a vector */
static inline void
ik_vec3_set(ikreal v[3], ikreal x, ikreal y, ikreal z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

/*! Set xyz components of vector to zero */
static inline void
ik_vec3_set_zero(ikreal v[3])
{
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
}

/*! Add scalar value to each vector component */
static inline void
ik_vec3_add_scalar(ikreal v[3], ikreal scalar)
{
    v[0] += scalar;
    v[1] += scalar;
    v[2] += scalar;
}

/*! Add v2 to v1 */
static inline void
ik_vec3_add_vec3(ikreal v1[3], const ikreal v2[3])
{
    v1[0] += v2[0];
    v1[1] += v2[1];
    v1[2] += v2[2];
}

/*! Subtract scalar value from each vector component */
static inline void
ik_vec3_sub_scalar(ikreal v[3], ikreal scalar)
{
    v[0] -= scalar;
    v[1] -= scalar;
    v[2] -= scalar;
}

/*! Subtract v2 from v1 */
static inline void
ik_vec3_sub_vec3(ikreal v1[3], const ikreal v2[3])
{
    v1[0] -= v2[0];
    v1[1] -= v2[1];
    v1[2] -= v2[2];
}

/*! Scale vector by a scalar value */
static inline void
ik_vec3_mul_scalar(ikreal v[3], ikreal scalar)
{
    v[0] *= scalar;
    v[1] *= scalar;
    v[2] *= scalar;
}

/*! Multiply v1 component-wise with v2 */
static inline void
ik_vec3_mul_vec3(ikreal v1[3], const ikreal v2[3])
{
    v1[0] *= v2[0];
    v1[1] *= v2[1];
    v1[2] *= v2[2];
}

/*! Scale vector by 1/scalar */
static inline void
ik_vec3_div_scalar(ikreal v[3], ikreal scalar)
{
    ikreal det = 1.0 / scalar;
    v[0] *= det;
    v[1] *= det;
    v[2] *= det;
}

/*! Divide v1 component-wise by v2 */
static inline void
ik_vec3_div_vec3(ikreal v1[3], const ikreal v2[3])
{
    v1[0] /= v2[0];
    v1[1] /= v2[1];
    v1[2] /= v2[2];
}

/*! Negate each vector component, 1-v */
static inline void
ik_vec3_negate(ikreal v[3])
{
    v[0] = -v[0];
    v[1] = -v[1];
    v[2] = -v[2];
}

/*! Calculate dot product of v1 and v2 */
static inline ikreal
ik_vec3_dot(const ikreal v1[3], const ikreal v2[3])
{
    return v1[0] * v2[0] +
           v1[1] * v2[1] +
           v1[2] * v2[2];
}

/*! Compute squared length of a vector */
static inline ikreal
ik_vec3_length_squared(const ikreal v[3])
{
    return ik_vec3_dot(v, v);
}

/*! Compute length of a vector */
static inline ikreal
ik_vec3_length(const ikreal v[3])
{
    return sqrt(ik_vec3_length_squared(v));
}

/*! Normalize vector (length is scaled to 1) */
static inline int
ik_vec3_normalize(ikreal v[3])
{
    ikreal length_squared = ik_vec3_length_squared(v);
    if (length_squared != 0.0)
    {
        ikreal length = 1.0 / sqrt(length_squared);
        v[0] *= length;
        v[1] *= length;
        v[2] *= length;
        return 1;
    }
    else
    {
        return 0;
    }
}

/*!
 * Calculate cross product v1 x v2. Store result into v1. If instead you want
 * to store the result in v2, see ik_vec3_ncross.
 */
static inline void
ik_vec3_cross(ikreal v1[3], const ikreal v2[3])
{
    ikreal v1x = v1[1] * v2[2] - v2[1] * v1[2];
    ikreal v1z = v1[0] * v2[1] - v2[0] * v1[1];
    v1[1]      = v1[2] * v2[0] - v2[2] * v1[0];
    v1[0] = v1x;
    v1[2] = v1z;
}

/*! Calculate cross product of v2 x v1. Store result into v1. */
static inline void
ik_vec3_ncross(ikreal v1[3], const ikreal v2[3])
{
    ikreal v1x = v2[1] * v1[2] - v1[1] * v2[2];
    ikreal v1z = v2[0] * v1[1] - v1[0] * v2[1];
    v1[1]      = v2[2] * v1[0] - v1[2] * v2[0];
    v1[0] = v1x;
    v1[2] = v1z;
}

/*! Rotates vector v by the quaternion q. */
static inline void
ik_vec3_rotate_quat(ikreal v[3], const ikreal q[4])
{
    /* v' = q * v * q' */
    /* https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion */
    union ik_vec3 tmp;
    ikreal dot_qv = ik_vec3_dot(q, v);
    ikreal dot_qq = ik_vec3_dot(q, q);
    ik_vec3_copy(tmp.f, v);
    /* 2.0f * s * cross(u, v) */
    ik_vec3_ncross(v, q);
    ik_vec3_mul_scalar(v, 2.0 * q[3]);
    /* + (s*s - dot(u, u)) * v */
    ik_vec3_mul_scalar(tmp.f, q[3]*q[3] - dot_qq);
    ik_vec3_add_vec3(v, tmp.f);
    /* + 2.0f * dot(u, v) * u */
    ik_vec3_copy(tmp.f, q);
    ik_vec3_mul_scalar(tmp.f, 2.0 * dot_qv);
    ik_vec3_add_vec3(v, tmp.f);
}

/*! Rotates vector v by the conjugate of the quaternion q. */
static inline void
ik_vec3_rotate_quat_conj(ikreal v[3], const ikreal q[4])
{
    /* v' = q * v * q' */
    /* more optimal: https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion */
    union ik_vec3 tmp;
    ikreal dot_qv = -ik_vec3_dot(q, v);
    ikreal dot_qq = ik_vec3_dot(q, q);
    ik_vec3_copy(tmp.f, v);
    /* 2.0f * s * cross(u, v) */
    ik_vec3_cross(v, q);
    ik_vec3_mul_scalar(v, 2.0 * q[3]);
    /* + (s*s - dot(u, u)) * v */
    ik_vec3_mul_scalar(tmp.f, q[3]*q[3] - dot_qq);
    ik_vec3_add_vec3(v, tmp.f);
    /* + 2.0f * dot(u, v) * u */
    ik_vec3_copy(tmp.f, q);
    ik_vec3_mul_scalar(tmp.f, 2.0 * dot_qv);
    ik_vec3_sub_vec3(v, tmp.f);
}

/*!
 * Rotates vector v by the angle between v1 and v2, assuming v1 and v2 are
 * unit vectors
 */
static inline void
ik_vec3_rotate_vec3_span_normalized(ikreal v[3], const ikreal v1[3], const ikreal v2[3])
{
    /* Rodrigues' rotation formula */
    /* vrot = v cos(a) + (k x v)sin(a) + k(k.v)(1-cos(a)) */
    union ik_vec3 k1, k2;
    ikreal a, cosa;

    /* angle between v1 and v2 */
    cosa = ik_vec3_dot(v1, v2);
    a = acos(cosa);

    /* axis of rotation */
    ik_vec3_copy(k1.f, v1);
    ik_vec3_cross(k1.f, v2);
    ik_vec3_copy(k2.f, k1.f);

    /* (k x v)sin(a) */
    ik_vec3_cross(k1.f, v);
    ik_vec3_mul_scalar(k1.f, sin(a));

    /* k(k.v)(1-cos(a)) */
    ik_vec3_mul_scalar(k2.f, ik_vec3_dot(k2.f, v));
    ik_vec3_mul_scalar(k2.f, 1.0 - cosa);

    /* accumulate */
    ik_vec3_mul_scalar(v, cosa);
    ik_vec3_add_vec3(v, k1.f);
    ik_vec3_add_vec3(v, k2.f);
}

/*! Rotates vector v by the angle between v1 and v2 */
static inline void
ik_vec3_rotate_vec3_span(ikreal v[3], const ikreal v1[3], const ikreal v2[3])
{
    union ik_vec3 v1n;
    union ik_vec3 v2n;
    ik_vec3_copy(v1n.f, v1);
    ik_vec3_copy(v2n.f, v2);
    ik_vec3_normalize(v1n.f);
    ik_vec3_normalize(v2n.f);
    ik_vec3_rotate_vec3_span_normalized(v, v1, v2);
}

/*! Projects v2 onto v1 */
static inline void
ik_vec3_project_from_vec3(ikreal v1[3], const ikreal v2[3])
{
    ikreal dot = ik_vec3_dot(v1, v2);
    ikreal det = ik_vec3_length_squared(v1);
    ik_vec3_mul_scalar(v1, dot / det);
}

/*! Projects v2 onto v1 and assumes both v1 and v2 are normalized. */
static inline void
ik_vec3_project_from_vec3_normalized(ikreal v1[3], const ikreal v2[3])
{
    ikreal dot = ik_vec3_dot(v1, v2);
    ik_vec3_mul_scalar(v1, dot);
}

/*! Projects v into the plane spanned by x,y. */
static inline void
ik_vec3_project_onto_plane(ikreal v[3], const ikreal x[3], const ikreal y[3])
{
    union ik_vec3 n;
    ik_vec3_copy(n.f, x);
    ik_vec3_cross(n.f, y);              /* plane normal */
    ik_vec3_project_from_vec3(n.f, v);  /* project vector onto normal */
    ik_vec3_sub_vec3(v, n.f);           /* subtract projection from vector */
}

C_END
