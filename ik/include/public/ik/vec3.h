#ifndef VEC3_H
#define VEC3_H

#include "ik/config.h"

C_BEGIN

union ik_vec3_t
{
    struct {
        ikreal_t x;
        ikreal_t y;
        ikreal_t z;
    } v;
    ikreal_t f[3];
};

#if defined(IK_BUILDING)

/*! Copy vector src into v */
IK_PRIVATE_API void
ik_vec3_copy(ikreal_t v[3], const ikreal_t src[3]);

/*! Set xyz components of a vector */
IK_PRIVATE_API void
ik_vec3_set(ikreal_t v[3], ikreal_t x, ikreal_t y, ikreal_t z);

/*! Set xyz components of vector to zero */
IK_PRIVATE_API void
ik_vec3_set_zero(ikreal_t v[3]);

/*! Add scalar value to each vector component */
IK_PRIVATE_API void
ik_vec3_add_scalar(ikreal_t v1[3], ikreal_t scalar);

/*! Add v2 to v1 */
IK_PRIVATE_API void
ik_vec3_add_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

/*! Subtract scalar value from each vector component */
IK_PRIVATE_API void
ik_vec3_sub_scalar(ikreal_t v1[3], ikreal_t scalar);

/*! Subtract v2 from v1 */
IK_PRIVATE_API void
ik_vec3_sub_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

/*! Scale vector by a scalar value */
IK_PRIVATE_API void
ik_vec3_mul_scalar(ikreal_t v1[3], ikreal_t scalar);

/*! Multiply v1 component-wise with v2 */
IK_PRIVATE_API void
ik_vec3_mul_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

/*! Scale vector by 1/scalar */
IK_PRIVATE_API void
ik_vec3_div_scalar(ikreal_t v[3], ikreal_t scalar);

/*! Divide v1 component-wise by v2 */
IK_PRIVATE_API void
ik_vec3_div_vec3(ikreal_t v[3], const ikreal_t v2[3]);

/*! Compute squared length of a vector */
IK_PRIVATE_API ikreal_t
ik_vec3_length_squared(const ikreal_t v[3]);

/*! Compute length of a vector */
IK_PRIVATE_API ikreal_t
ik_vec3_length(const ikreal_t v[3]);

/*! Normalize vector (length is scaled to 1) */
IK_PRIVATE_API void
ik_vec3_normalize(ikreal_t v[3]);

/*! Calculate dot product of v1 and v2 */
IK_PRIVATE_API ikreal_t
ik_vec3_dot(const ikreal_t v1[3], const ikreal_t v2[3]);

/*!
 * Calculate cross product v1 x v2. Store result into v1. If instead you want
 * to store the result in v2, see ik_vec3_ncross.
 */
IK_PRIVATE_API void
ik_vec3_cross(ikreal_t v1[3], const ikreal_t v2[3]);

/*! Calculate cross product of v2 x v1. Store result into v1. */
IK_PRIVATE_API void
ik_vec3_ncross(ikreal_t v1[3], const ikreal_t v2[3]);

/*! Rotates vector v by the quaternion q. */
IK_PRIVATE_API void
ik_vec3_rotate_quat(ikreal_t v[3], const ikreal_t q[4]);

/*! Rotates vector v by the conjugate of the quaternion q. */
IK_PRIVATE_API void
ik_vec3_nrotate_quat(ikreal_t v[3], const ikreal_t q[4]);

/*! Rotates vector v by the angle between v1 and v2 */
IK_PRIVATE_API void
ik_vec3_rotate_vec3_span(ikreal_t v[3], const ikreal_t v1[3], const ikreal_t v2[3]);

/*!
 * Rotates vector v by the angle between v1 and v2, assuming v1 and v2 are
 * unit vectors
 */
IK_PRIVATE_API void
ik_vec3_rotate_vec3_span_normalized(ikreal_t v[3], const ikreal_t v1[3], const ikreal_t v2[3]);

/*! Projects v2 onto v1 */
IK_PRIVATE_API void
ik_vec3_project_from_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

/*! Projects v2 onto v1 and assumes both v1 and v2 are normalized. */
IK_PRIVATE_API void
ik_vec3_project_from_vec3_normalized(ikreal_t v1[3], const ikreal_t v2[3]);

/*! Projects v into the plane spanned by x,y. */
IK_PRIVATE_API void
ik_vec3_project_onto_plane(ikreal_t v[3], const ikreal_t x[3], const ikreal_t y[3]);

#endif /* IK_BUILDING */

C_END

#endif /* VEC3_H */
