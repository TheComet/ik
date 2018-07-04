#include "ik/vec3_static.h"
#include "ik/quat_static.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------------- */
ik_vec3_t
ik_vec3_static_vec3(ikreal_t x, ikreal_t y, ikreal_t z)
{
    ik_vec3_t ret;
    ret.x = x;
    ret.y = y;
    ret.z = z;
    return ret;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_set_zero(ikreal_t v[3])
{
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_set(ikreal_t v[3], const ikreal_t src[3])
{
    v[0] = src[0];
    v[1] = src[1];
    v[2] = src[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_add_scalar(ikreal_t v[3], ikreal_t scalar)
{
    v[0] += scalar;
    v[1] += scalar;
    v[2] += scalar;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_add_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] += v2[0];
    v1[1] += v2[1];
    v1[2] += v2[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_sub_scalar(ikreal_t v[3], ikreal_t scalar)
{
    v[0] -= scalar;
    v[1] -= scalar;
    v[2] -= scalar;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_sub_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] -= v2[0];
    v1[1] -= v2[1];
    v1[2] -= v2[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_mul_scalar(ikreal_t v[3], ikreal_t scalar)
{
    v[0] *= scalar;
    v[1] *= scalar;
    v[2] *= scalar;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_mul_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] *= v2[0];
    v1[1] *= v2[1];
    v1[2] *= v2[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_div_scalar(ikreal_t v[3], ikreal_t scalar)
{
    ikreal_t det = 1.0 / scalar;
    v[0] *= det;
    v[1] *= det;
    v[2] *= det;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_div_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] /= v2[0];
    v1[1] /= v2[1];
    v1[2] /= v2[2];
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_vec3_static_length_squared(const ikreal_t v[3])
{
    return ik_vec3_static_dot(v, v);
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_vec3_static_length(const ikreal_t v[3])
{
    return sqrt(ik_vec3_static_length_squared(v));
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_normalize(ikreal_t v[3])
{
    ikreal_t length = ik_vec3_static_length(v);
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
ikreal_t
ik_vec3_static_dot(const ikreal_t v1[3], const ikreal_t v2[3])
{
    return v1[0] * v2[0] +
           v1[1] * v2[1] +
           v1[2] * v2[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_static_cross(ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t v1x = v1[1] * v2[2] - v2[1] * v1[2];
    ikreal_t v1z = v1[0] * v2[1] - v2[0] * v1[1];
    v1[1]       = v1[2] * v2[0] - v2[2] * v1[0];
    v1[0] = v1x;
    v1[2] = v1z;
}

/* ------------------------------------------------------------------------- */
static void
mul_quat_no_normalize(ikreal_t q1[4], const ikreal_t q2[4])
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
ik_vec3_static_rotate(ikreal_t v[3], const ikreal_t q[4])
{
    /* P' = RPR' */
    ik_quat_t result;
    ik_quat_t conj;
    ik_quat_t point;

    ik_vec3_static_set(point.f, v);
    point.w = 0.0;

    ik_quat_static_set(conj.f, q);
    ik_quat_static_conj(conj.f);

    ik_quat_static_set(result.f, q);
    mul_quat_no_normalize(result.f, point.f);
    mul_quat_no_normalize(result.f, conj.f);
    ik_vec3_static_set(v, result.f);
}
