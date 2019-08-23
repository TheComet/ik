#include "ik/vec3.h"
#include <string.h>
#include <math.h>

/* ------------------------------------------------------------------------- */
void
ik_vec3_set(ikreal_t v[3], ikreal_t x, ikreal_t y, ikreal_t z)
{
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_set_zero(ikreal_t v[3])
{
    v[0] = 0.0;
    v[1] = 0.0;
    v[2] = 0.0;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_copy(ikreal_t v[3], const ikreal_t src[3])
{
    v[0] = src[0];
    v[1] = src[1];
    v[2] = src[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_add_scalar(ikreal_t v[3], ikreal_t scalar)
{
    v[0] += scalar;
    v[1] += scalar;
    v[2] += scalar;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_add_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] += v2[0];
    v1[1] += v2[1];
    v1[2] += v2[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_sub_scalar(ikreal_t v[3], ikreal_t scalar)
{
    v[0] -= scalar;
    v[1] -= scalar;
    v[2] -= scalar;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_sub_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] -= v2[0];
    v1[1] -= v2[1];
    v1[2] -= v2[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_mul_scalar(ikreal_t v[3], ikreal_t scalar)
{
    v[0] *= scalar;
    v[1] *= scalar;
    v[2] *= scalar;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_mul_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] *= v2[0];
    v1[1] *= v2[1];
    v1[2] *= v2[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_div_scalar(ikreal_t v[3], ikreal_t scalar)
{
    ikreal_t det = 1.0 / scalar;
    v[0] *= det;
    v[1] *= det;
    v[2] *= det;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_div_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    v1[0] /= v2[0];
    v1[1] /= v2[1];
    v1[2] /= v2[2];
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_vec3_length_squared(const ikreal_t v[3])
{
    return ik_vec3_dot(v, v);
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_vec3_length(const ikreal_t v[3])
{
    return sqrt(ik_vec3_length_squared(v));
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_normalize(ikreal_t v[3])
{
    ikreal_t length = ik_vec3_length(v);
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
ik_vec3_dot(const ikreal_t v1[3], const ikreal_t v2[3])
{
    return v1[0] * v2[0] +
           v1[1] * v2[1] +
           v1[2] * v2[2];
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_cross(ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t v1x = v1[1] * v2[2] - v2[1] * v1[2];
    ikreal_t v1z = v1[0] * v2[1] - v2[0] * v1[1];
    v1[1]        = v1[2] * v2[0] - v2[2] * v1[0];
    v1[0] = v1x;
    v1[2] = v1z;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_ncross(ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t v1x = v2[1] * v1[2] - v1[1] * v2[2];
    ikreal_t v1z = v2[0] * v1[1] - v1[0] * v2[1];
    v1[1]        = v2[2] * v1[0] - v1[2] * v2[0];
    v1[0] = v1x;
    v1[2] = v1z;
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_rotate_quat(ikreal_t v[3], const ikreal_t q[4])
{
    /* v' = q * v * q' */
    /* more optimal: https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion */
    union ik_vec3_t tmp;
    ikreal_t dot_qv = ik_vec3_dot(q, v);
    ikreal_t dot_qq = ik_vec3_dot(q, q);
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

/* ------------------------------------------------------------------------- */
void
ik_vec3_nrotate_quat(ikreal_t v[3], const ikreal_t q[4])
{
    /* v' = q * v * q' */
    /* more optimal: https://gamedev.stackexchange.com/questions/28395/rotating-vector3-by-a-quaternion */
    union ik_vec3_t tmp;
    ikreal_t dot_qv = -ik_vec3_dot(q, v);
    ikreal_t dot_qq = ik_vec3_dot(q, q);
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

/* ------------------------------------------------------------------------- */
void
ik_vec3_rotate_vec3_span(ikreal_t v[3], const ikreal_t v1[3], const ikreal_t v2[3])
{
    union ik_vec3_t v1n;
    union ik_vec3_t v2n;
    ik_vec3_copy(v1n.f, v1);
    ik_vec3_copy(v2n.f, v2);
    ik_vec3_normalize(v1n.f);
    ik_vec3_normalize(v2n.f);
    ik_vec3_rotate_vec3_span_normalized(v, v1, v2);
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_rotate_vec3_span_normalized(ikreal_t v[3], const ikreal_t v1[3], const ikreal_t v2[3])
{
    /* Rodrigues' rotation formula */
    /* vrot = v cos(a) + (k x v)sin(a) + k(k.v)(1-cos(a)) */
    union ik_vec3_t k1, k2;
    ikreal_t a, cosa;

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

/* ------------------------------------------------------------------------- */
void
ik_vec3_project_from_vec3(ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t dot = ik_vec3_dot(v1, v2);
    ikreal_t det = ik_vec3_length_squared(v1);
    ik_vec3_mul_scalar(v1, dot / det);
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_project_from_vec3_normalized(ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t dot = ik_vec3_dot(v1, v2);
    ik_vec3_mul_scalar(v1, dot);
}

/* ------------------------------------------------------------------------- */
void
ik_vec3_project_onto_plane(ikreal_t v[3], const ikreal_t x[3], const ikreal_t y[3])
{
    union ik_vec3_t n;
    ik_vec3_copy(n.f, x);
    ik_vec3_cross(n.f, y);              /* plane normal */
    ik_vec3_project_from_vec3(n.f, v);  /* project vector onto normal */
    ik_vec3_sub_vec3(v, n.f);           /* subtract projection from vector */
}
