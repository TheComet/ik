#include "ik/quat_static.h"
#include "ik/vec3_static.h"
#include <math.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
ik_quat_t
ik_quat_static_quat(ikreal_t x, ikreal_t y, ikreal_t z, ikreal_t w)
{
    ik_quat_t ret;
    ret.x = x;
    ret.y = y;
    ret.z = z;
    ret.w = w;
    return ret;
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_set_identity(ikreal_t q[4])
{
    memset(q, 0, sizeof(ikreal_t) * 3);
    q[3] = 1;
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_set(ikreal_t q[4], const ikreal_t src[4])
{
    q[0] = src[0];
    q[1] = src[1];
    q[2] = src[2];
    q[3] = src[3];
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_add_quat(ikreal_t q1[4], const ikreal_t q2[4])
{
    q1[0] += q2[0];
    q1[1] += q2[1];
    q1[2] += q2[2];
    q1[3] += q2[3];
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_quat_static_mag(const ikreal_t q[4])
{
    return sqrt(q[3]*q[3] + q[2]*q[2] + q[1]*q[1] + q[0]*q[0]);
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_conj(ikreal_t q[4])
{
    q[0] = -q[0];
    q[1] = -q[1];
    q[2] = -q[2];
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_invert_sign(ikreal_t q[4])
{
    q[0] = -q[0];
    q[1] = -q[1];
    q[2] = -q[2];
    q[3] = -q[3];
}


/* ------------------------------------------------------------------------- */
void
ik_quat_static_normalize(ikreal_t q[4])
{
    ikreal_t mag = ik_quat_static_mag(q);
    if (mag != 0.0)
        mag = 1.0 / mag;
    q[0] *= mag;
    q[1] *= mag;
    q[2] *= mag;
    q[3] *= mag;
}

/* ------------------------------------------------------------------------- */
static void
mul_ik_quat_static_no_normalize(ikreal_t q1[4], const ikreal_t q2[4])
{
    ik_vec3_t v1;
    ik_vec3_t v2;
    ik_vec3_static_set(v1.f, q1);
    ik_vec3_static_set(v2.f, q2);

    ik_vec3_static_mul_scalar(v1.f, q2[3]);
    ik_vec3_static_mul_scalar(v2.f, q1[3]);
    q1[3] = q1[3]*q2[3] - ik_vec3_static_dot(q1, q2);
    ik_vec3_static_cross(q1, q2);
    ik_vec3_static_add_vec3(q1, v1.f);
    ik_vec3_static_add_vec3(q1, v2.f);
}
void
ik_quat_static_mul_quat(ikreal_t q1[4], const ikreal_t q2[4])
{
    mul_ik_quat_static_no_normalize(q1, q2);
    ik_quat_static_normalize(q1);
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_mul_scalar(ikreal_t q[4], ikreal_t scalar)
{
    q[0] *= scalar;
    q[1] *= scalar;
    q[2] *= scalar;
    q[3] *= scalar;
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_div_scalar(ikreal_t q[4], ikreal_t scalar)
{
    if (scalar == 0.0)
        ik_quat_static_set_identity(q);
    else
    {
        ikreal_t rec = 1.0 / scalar;
        q[0] *= rec;
        q[1] *= rec;
        q[2] *= rec;
        q[3] *= rec;
    }
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_quat_static_dot(const ikreal_t q1[4], const ikreal_t q2[4])
{
    return q1[0] * q2[0] +
           q1[1] * q2[1] +
           q1[2] * q2[2] +
           q1[3] * q2[3];
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_normalize_sign(ikreal_t q1[4])
{
    ik_quat_t unit = {{0, 0, 0, 1}};
    ikreal_t dot = ik_quat_static_dot(q1, unit.f);
    if (dot < 0.0)
        ik_quat_static_invert_sign(q1);
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_angle_unnormalized(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t cos_a, sin_a, angle, denominator;

    denominator = 1.0 / ik_vec3_static_length(v1) / ik_vec3_static_length(v2);
    cos_a = ik_vec3_static_dot(v1, v2) * denominator;
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        memcpy(q, v1, sizeof(ikreal_t) * 3);
        ik_vec3_static_cross(q, v2);
        ik_vec3_static_normalize(q);

        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(cos_a);
        cos_a = cos(angle * 0.5);
        sin_a = sin(angle * 0.5);
        ik_vec3_static_mul_scalar(q, sin_a);
        q[3] = cos_a; /* w component */
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        ik_quat_static_set_identity(q);
    }
}

/* ------------------------------------------------------------------------- */
void
ik_quat_static_angle(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t cos_a, sin_a, angle;

    cos_a = ik_vec3_static_dot(v1, v2);
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        ik_vec3_static_set(q, v1);
        ik_vec3_static_cross(q, v2);
        /* would usually normalizehere, but cross product of two normalized
         * vectors is already normalized*/

        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(cos_a);
        cos_a = cos(angle * 0.5);
        sin_a = sin(angle * 0.5);
        ik_vec3_static_mul_scalar(q, sin_a);
        q[3] = cos_a; /* w component */
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        ik_quat_static_set_identity(q);
    }
}
