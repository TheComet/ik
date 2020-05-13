#ifndef VEC3_H
#define VEC3_H

#include "ik/config.h"

C_BEGIN

union ik_vec3
{
    struct {
        ikreal x;
        ikreal y;
        ikreal z;
    } v;
    ikreal f[3];
};

/*! Copy vector src into v */
IK_PUBLIC_API void
ik_vec3_copy(ikreal v[3], const ikreal src[3]);

/*! Set xyz components of a vector */
IK_PUBLIC_API void
ik_vec3_set(ikreal v[3], ikreal x, ikreal y, ikreal z);

/*! Set xyz components of vector to zero */
IK_PUBLIC_API void
ik_vec3_set_zero(ikreal v[3]);

/*! Add scalar value to each vector component */
IK_PUBLIC_API void
ik_vec3_add_scalar(ikreal v1[3], ikreal scalar);

/*! Add v2 to v1 */
IK_PUBLIC_API void
ik_vec3_add_vec3(ikreal v1[3], const ikreal v2[3]);

/*! Subtract scalar value from each vector component */
IK_PUBLIC_API void
ik_vec3_sub_scalar(ikreal v1[3], ikreal scalar);

/*! Subtract v2 from v1 */
IK_PUBLIC_API void
ik_vec3_sub_vec3(ikreal v1[3], const ikreal v2[3]);

/*! Scale vector by a scalar value */
IK_PUBLIC_API void
ik_vec3_mul_scalar(ikreal v1[3], ikreal scalar);

/*! Multiply v1 component-wise with v2 */
IK_PUBLIC_API void
ik_vec3_mul_vec3(ikreal v1[3], const ikreal v2[3]);

/*! Scale vector by 1/scalar */
IK_PUBLIC_API void
ik_vec3_div_scalar(ikreal v[3], ikreal scalar);

/*! Divide v1 component-wise by v2 */
IK_PUBLIC_API void
ik_vec3_div_vec3(ikreal v[3], const ikreal v2[3]);

/*! Compute squared length of a vector */
IK_PUBLIC_API ikreal
ik_vec3_length_squared(const ikreal v[3]);

/*! Compute length of a vector */
IK_PUBLIC_API ikreal
ik_vec3_length(const ikreal v[3]);

/*! Normalize vector (length is scaled to 1) */
IK_PUBLIC_API int
ik_vec3_normalize(ikreal v[3]);

/*! Calculate dot product of v1 and v2 */
IK_PUBLIC_API ikreal
ik_vec3_dot(const ikreal v1[3], const ikreal v2[3]);

/*!
 * Calculate cross product v1 x v2. Store result into v1. If instead you want
 * to store the result in v2, see ik_vec3_ncross.
 */
IK_PUBLIC_API void
ik_vec3_cross(ikreal v1[3], const ikreal v2[3]);

/*! Calculate cross product of v2 x v1. Store result into v1. */
IK_PUBLIC_API void
ik_vec3_ncross(ikreal v1[3], const ikreal v2[3]);

/*! Rotates vector v by the quaternion q. */
IK_PUBLIC_API void
ik_vec3_rotate_quat(ikreal v[3], const ikreal q[4]);

/*! Rotates vector v by the conjugate of the quaternion q. */
IK_PUBLIC_API void
ik_vec3_nrotate_quat(ikreal v[3], const ikreal q[4]);

/*! Rotates vector v by the angle between v1 and v2 */
IK_PUBLIC_API void
ik_vec3_rotate_vec3_span(ikreal v[3], const ikreal v1[3], const ikreal v2[3]);

/*!
 * Rotates vector v by the angle between v1 and v2, assuming v1 and v2 are
 * unit vectors
 */
IK_PUBLIC_API void
ik_vec3_rotate_vec3_span_normalized(ikreal v[3], const ikreal v1[3], const ikreal v2[3]);

/*! Projects v2 onto v1 */
IK_PUBLIC_API void
ik_vec3_project_from_vec3(ikreal v1[3], const ikreal v2[3]);

/*! Projects v2 onto v1 and assumes both v1 and v2 are normalized. */
IK_PUBLIC_API void
ik_vec3_project_from_vec3_normalized(ikreal v1[3], const ikreal v2[3]);

/*! Projects v into the plane spanned by x,y. */
IK_PUBLIC_API void
ik_vec3_project_onto_plane(ikreal v[3], const ikreal x[3], const ikreal y[3]);

C_END

#endif /* VEC3_H */
