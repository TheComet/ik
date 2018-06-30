#include "ik/vec3.h"
#include "ik/quat.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------------- */
void
vec3_set_zero(ikreal_t v[3])
{
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
}

/* ------------------------------------------------------------------------- */
void
vec3_set (ikreal_t v[3], const ikreal_t src[3])
{
    v[0] = src[0];
    v[1] = src[1];
    v[2] = src[2];
}

/* ------------------------------------------------------------------------- */
void
vec3_add_scalar(ikreal_t v[3], ikreal_t scalar)
{
    v[0] += scalar;
    v[1] += scalar;
    v[2] += scalar;
}

/* ------------------------------------------------------------------------- */
void
vec3_add_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] += v2[0];
    v1[1] += v2[1];
    v1[2] += v2[2];
}

/* ------------------------------------------------------------------------- */
void
vec3_sub_scalar(ikreal_t v[3], ikreal_t scalar)
{
    v[0] -= scalar;
    v[1] -= scalar;
    v[2] -= scalar;
}

/* ------------------------------------------------------------------------- */
void
vec3_sub_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] -= v2[0];
    v1[1] -= v2[1];
    v1[2] -= v2[2];
}

/* ------------------------------------------------------------------------- */
void
vec3_mul_scalar(ikreal_t v[3], ikreal_t scalar)
{
    v[0] *= scalar;
    v[1] *= scalar;
    v[2] *= scalar;
}

/* ------------------------------------------------------------------------- */
void
vec3_mul_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] *= v2[0];
    v1[1] *= v2[1];
    v1[2] *= v2[2];
}

/* ------------------------------------------------------------------------- */
void
vec3_div_scalar(ikreal_t v[3], ikreal_t scalar)
{
    ikreal_t det = 1.0 / scalar;
    v[0] *= det;
    v[1] *= det;
    v[2] *= det;
}

/* ------------------------------------------------------------------------- */
void
vec3_div_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] /= v2[0];
    v1[1] /= v2[1];
    v1[2] /= v2[2];
}

/* ------------------------------------------------------------------------- */
ikreal_t
vec3_length_squared(const ikreal_t v[3])
{
    return vec3_dot(v, v);
}

/* ------------------------------------------------------------------------- */
ikreal_t
vec3_length(const ikreal_t v[3])
{
    return sqrt(vec3_length_squared(v));
}

/* ------------------------------------------------------------------------- */
void
vec3_normalize(ikreal_t v[3])
{
    ikreal_t length = vec3_length(v);
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
vec3_dot(const ikreal_t v1[3], const ikreal_t v2[3])
{
    return v1[0] * v2[0] +
           v1[1] * v2[1] +
           v1[2] * v2[2];
}

/* ------------------------------------------------------------------------- */
void
vec3_cross(ikreal_t v1[3], const ikreal_t v2[3])
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
    ikreal_t v1[3];
    ikreal_t v2[3];
    memcpy(v1, q1, sizeof(ikreal_t) * 3);
    memcpy(v2, q2, sizeof(ikreal_t) * 3);

    vec3_mul_scalar(v1, q2[3]);
    vec3_mul_scalar(v2, q1[3]);
    q1[3] = q1[3]*q2[3] - vec3_dot(q1, q2);
    vec3_cross(q1, q2);
    vec3_add_vec3(q1, v1);
    vec3_add_vec3(q1, v2);
}
void
vec3_rotate(ikreal_t v[3], const ikreal_t q[4])
{
    /* P' = RPR' */
    quat_t result;
    quat_t conj;
    quat_t point;

    memcpy(point.f, v, sizeof(ikreal_t) * 3);
    point.w = 0.0;

    conj = *(quat_t*)q;
    quat_conj(conj.f);

    result = *(quat_t*)q;
    mul_quat_no_normalize(result.f, point.f);
    mul_quat_no_normalize(result.f, conj.f);
    memcpy(v, result.f, sizeof(ikreal_t) * 3);
}
