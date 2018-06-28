#include "ik/vec3.h"
#include "ik/quat.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------------- */
void
vec3_set_zero(ik_real v[3])
{
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
}

/* ------------------------------------------------------------------------- */
void
vec3_copy(ik_real v[3], const ik_real src[3])
{
    v[0] = src[0];
    v[1] = src[1];
    v[2] = src[2];
}

/* ------------------------------------------------------------------------- */
void
vec3_add_vec3(ik_real v1[3], const ik_real v2[3])
{
    v1[0] += v2[0];
    v1[1] += v2[1];
    v1[2] += v2[2];
}

/* ------------------------------------------------------------------------- */
void
vec3_sub_vec3(ik_real v1[3], const ik_real v2[3])
{
    v1[0] -= v2[0];
    v1[1] -= v2[1];
    v1[2] -= v2[2];
}

/* ------------------------------------------------------------------------- */
void
vec3_mul_scalar(ik_real v[3], ik_real scalar)
{
    v[0] *= scalar;
    v[1] *= scalar;
    v[2] *= scalar;
}

/* ------------------------------------------------------------------------- */
void
vec3_div_scalar(ik_real v[3], ik_real scalar)
{
    v[0] /= scalar;
    v[1] /= scalar;
    v[2] /= scalar;
}

/* ------------------------------------------------------------------------- */
ik_real
vec3_length_squared(const ik_real v[3])
{
    return vec3_dot(v, v);
}

/* ------------------------------------------------------------------------- */
ik_real
vec3_length(const ik_real v[3])
{
    return sqrt(vec3_length_squared(v));
}

/* ------------------------------------------------------------------------- */
void
vec3_normalize(ik_real v[3])
{
    ik_real length = vec3_length(v);
    if (length != 0.0)
    {
        length = 1.0 / length;
        v[0] *= length;
        v[1] *= length;
        v[2] *= length;
    }
    else
    {
        v[0] = 1;
    }
}

/* ------------------------------------------------------------------------- */
ik_real
vec3_dot(const ik_real v1[3], const ik_real v2[3])
{
    return v1[0] * v2[0] +
           v1[1] * v2[1] +
           v1[2] * v2[2];
}

/* ------------------------------------------------------------------------- */
void
vec3_cross(ik_real v1[3], const ik_real v2[3])
{
    ik_real v1x = v1[1] * v2[2] - v2[1] * v1[2];
    ik_real v1z = v1[0] * v2[1] - v2[0] * v1[1];
    v1[1]       = v1[2] * v2[0] - v2[2] * v1[0];
    v1[0] = v1x;
    v1[2] = v1z;
}

/* ------------------------------------------------------------------------- */
void
vec3_angle(ik_real q[4], const ik_real v1[3], const ik_real v2[3])
{
    ik_real cos_a, sin_a, angle, denominator;

    denominator = 1.0 / vec3_length(v1) / vec3_length(v2);
    cos_a = vec3_dot(v1, v2) * denominator;
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        memcpy(q, v1, sizeof(ik_real) * 3);
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
vec3_angle_normalized(ik_real q[4], const ik_real v1[3], const ik_real v2[3])
{
    ik_real cos_a, sin_a, angle;

    cos_a = vec3_dot(v1, v2);
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        vec3_copy(q, v1);
        vec3_cross(q, v2);
        /* would usually normalize here, but cross product of two normalized
         * vectors is already normalized */

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
