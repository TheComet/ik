#ifndef VEC3_H
#define VEC3_H

#include "ik/config.h"

C_BEGIN

struct ik_vec3_t
{
    union {
        struct {
            ikreal_t x;
            ikreal_t y;
            ikreal_t z;
        };
        ikreal_t f[3];
    };
};

void
ik_vec3_copy(ikreal_t v[3], const ikreal_t src[3]);

void
ik_vec3_set(ikreal_t v[3], ikreal_t x, ikreal_t y, ikreal_t z);

void
ik_vec3_set_zero(ikreal_t v[3]);

void
ik_vec3_add_scalar(ikreal_t v1[3], ikreal_t scalar);

void
ik_vec3_add_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

void
ik_vec3_sub_scalar(ikreal_t v1[3], ikreal_t scalar);

void
ik_vec3_sub_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

void
ik_vec3_mul_scalar(ikreal_t v1[3], ikreal_t scalar);

void
ik_vec3_mul_vec3(ikreal_t v1[3], const ikreal_t v2[3]);

void
ik_vec3_div_scalar(ikreal_t v[3], ikreal_t scalar);

void
ik_vec3_div_vec3(ikreal_t v[3], const ikreal_t v2[3]);

ikreal_t
ik_vec3_length_squared(const ikreal_t v[3]);

ikreal_t
ik_vec3_length(const ikreal_t v[3]);

void
ik_vec3_normalize(ikreal_t v[3]);

ikreal_t
ik_vec3_dot(const ikreal_t v1[3], const ikreal_t v2[3]);

void
ik_vec3_cross(ikreal_t v1[3], const ikreal_t v2[3]);

void
ik_vec3_ncross(ikreal_t v1[3], const ikreal_t v2[3]);

/*!
 * @brief Rotates a vector by the specified quaternion.
 */
void
ik_vec3_rotate(ikreal_t v[3], const ikreal_t q[4]);

/*!
 * @brief Rotations a vector by the conjugate of the specified quaternion.
 */
void
ik_vec3_nrotate(ikreal_t v[3], const ikreal_t q[4]);

/*!
 * @brief Projects v2 onto v1
 */
void
ik_vec3_project_from_vec3(ikreal_t v1[3], const ikreal_t v2[3]);


/*!
* @brief Projects v2 onto v1 and assumes both v1 and v2 are normalized.
*/
void
ik_vec3_project_from_vec3_normalized(ikreal_t v1[3], const ikreal_t v2[3]);

/*!
 * @brief Projects v into the plane spanned by x,y.
 */
void
ik_vec3_project_onto_plane(ikreal_t v[3], const ikreal_t x[3], const ikreal_t y[3]);


C_END

#endif /* VEC3_H */
