#include "ik/quat.h"
#include <math.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
void
quat_set_identity(ikreal_t q[4])
{
    memset(q, 0, sizeof(ikreal_t) * 3);
    q[3] = 1;
}

/* ------------------------------------------------------------------------- */
void
quat_set(ikreal_t q[4], const ikreal_t src[4])
{
    q[0] = src[0];
    q[1] = src[1];
    q[2] = src[2];
    q[3] = src[3];
}

/* ------------------------------------------------------------------------- */
void
quat_add_quat(ikreal_t q1[4], const ikreal_t q2[4])
{
    q1[0] += q2[0];
    q1[1] += q2[1];
    q1[2] += q2[2];
    q1[3] += q2[3];
}

/* ------------------------------------------------------------------------- */
ikreal_t
quat_mag(const ikreal_t q[4])
{
    return sqrt(q[3]*q[3] + q[2]*q[2] + q[1]*q[1] + q[0]*q[0]);
}

/* ------------------------------------------------------------------------- */
void
quat_conj(ikreal_t q[4])
{
    q[0] = -q[0];
    q[1] = -q[1];
    q[2] = -q[2];
}

/* ------------------------------------------------------------------------- */
void
quat_invert_sign(ikreal_t q[4])
{
    q[0] = -q[0];
    q[1] = -q[1];
    q[2] = -q[2];
    q[3] = -q[3];
}


/* ------------------------------------------------------------------------- */
void
quat_normalize(ikreal_t q[4])
{
    ikreal_t mag = quat_mag(q);
    if (mag != 0.0)
        mag = 1.0 / mag;
    q[0] *= mag;
    q[1] *= mag;
    q[2] *= mag;
    q[3] *= mag;
}

/* ------------------------------------------------------------------------- */
static void
mul_quat_no_normalize(ikreal_t q1[4], const ikreal_t q2[4])
{
    vec3_t v1;
    vec3_t v2;
    vec3_set(v1.f, q1);
    vec3_set(v2.f, q2);

    vec3_mul_scalar(v1.f, q2[3]);
    vec3_mul_scalar(v2.f, q1[3]);
    q1[3] = q1[3]*q2[3] - vec3_dot(q1, q2);
    vec3_cross(q1, q2);
    vec3_add_vec3(q1, v1.f);
    vec3_add_vec3(q1, v2.f);
}
void
quat_mul_quat(ikreal_t q1[4], const ikreal_t q2[4])
{
    mul_quat_no_normalize(q1, q2);
    quat_normalize(q1);
}

/* ------------------------------------------------------------------------- */
void
quat_mul_scalar(ikreal_t q[4], ikreal_t scalar)
{
    q[0] *= scalar;
    q[1] *= scalar;
    q[2] *= scalar;
    q[3] *= scalar;
}

/* ------------------------------------------------------------------------- */
void
quat_div_scalar(ikreal_t q[4], ikreal_t scalar)
{
    if (scalar == 0.0)
        quat_set_identity(q);
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
quat_dot(const ikreal_t q1[4], const ikreal_t q2[4])
{
    return q1[0] * q2[0] +
           q1[1] * q2[1] +
           q1[2] * q2[2] +
           q1[3] * q2[3];
}

/* ------------------------------------------------------------------------- */
void
quat_normalize_sign(ikreal_t q1[4])
{
    quat_t unit = {{0, 0, 0, 1}};
    ikreal_t dot = quat_dot(q1, unit.f);
    if (dot < 0.0)
        quat_invert_sign(q1);
}

/* ------------------------------------------------------------------------- */
void
quat_angle_unnormalized(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t cos_a, sin_a, angle, denominator;

    denominator = 1.0 / vec3_length(v1) / vec3_length(v2);
    cos_a = vec3_dot(v1, v2) * denominator;
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        memcpy(q, v1, sizeof(ikreal_t) * 3);
        vec3_cross(q, v2);
        vec3_normalize(q);

        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(cos_a);
        cos_a = cos(angle * 0.5);
        sin_a = sin(angle * 0.5);
        vec3_mul_scalar(q, sin_a);
        q[3] = cos_a; /* w component */
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        quat_set_identity(q);
    }
}

/* ------------------------------------------------------------------------- */
void
quat_angle(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t cos_a, sin_a, angle;

    cos_a = vec3_dot(v1, v2);
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        vec3_set(q, v1);
        vec3_cross(q, v2);
        /* would usually normalizehere, but cross product of two normalized
         * vectors is already normalized*/

        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(cos_a);
        cos_a = cos(angle * 0.5);
        sin_a = sin(angle * 0.5);
        vec3_mul_scalar(q, sin_a);
        q[3] = cos_a; /* w component */
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        quat_set_identity(q);
    }
}
