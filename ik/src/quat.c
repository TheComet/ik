#include "ik/quat.h"
#include "ik/vec3.h"
#include <math.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
void
ik_quat_set(ikreal_t q[4], ikreal_t x, ikreal_t y, ikreal_t z, ikreal_t w)
{
    q[0] = x;
    q[1] = y;
    q[2] = z;
    q[3] = w;
}

/* ------------------------------------------------------------------------- */
void
ik_quat_set_identity(ikreal_t q[4])
{
    memset(q, 0, sizeof(ikreal_t) * 3);
    q[3] = 1;
}

/* ------------------------------------------------------------------------- */
void
ik_quat_copy(ikreal_t q[4], const ikreal_t src[4])
{
    q[0] = src[0];
    q[1] = src[1];
    q[2] = src[2];
    q[3] = src[3];
}

/* ------------------------------------------------------------------------- */
void
ik_quat_set_axis_angle(ikreal_t q[4], ikreal_t x, ikreal_t y, ikreal_t z, ikreal_t a)
{
    ik_vec3_set(q, x, y, z);
    ik_vec3_normalize(q);
    ik_vec3_mul_scalar(q, sin(a * 0.5));
    q[3] = cos(a * 0.5);
}

/* ------------------------------------------------------------------------- */
void
ik_quat_add_quat(ikreal_t q1[4], const ikreal_t q2[4])
{
    q1[0] += q2[0];
    q1[1] += q2[1];
    q1[2] += q2[2];
    q1[3] += q2[3];
}

/* ------------------------------------------------------------------------- */
ikreal_t
ik_quat_mag(const ikreal_t q[4])
{
    return sqrt(q[3]*q[3] + q[2]*q[2] + q[1]*q[1] + q[0]*q[0]);
}

/* ------------------------------------------------------------------------- */
void
ik_quat_conj(ikreal_t q[4])
{
    q[0] = -q[0];
    q[1] = -q[1];
    q[2] = -q[2];
}

/* ------------------------------------------------------------------------- */
void
ik_quat_negate(ikreal_t q[4])
{
    q[0] = -q[0];
    q[1] = -q[1];
    q[2] = -q[2];
    q[3] = -q[3];
}

/* ------------------------------------------------------------------------- */
void
ik_quat_invert(ikreal_t q[4])
{
    ikreal_t mag_squared = ik_quat_dot(q, q);
    ik_quat_conj(q);
    ik_quat_div_scalar(q, mag_squared);
}

/* ------------------------------------------------------------------------- */
void
ik_quat_normalize(ikreal_t q[4])
{
    ikreal_t r_mag = 1.0 / ik_quat_mag(q);
    q[0] *= r_mag;
    q[1] *= r_mag;
    q[2] *= r_mag;
    q[3] *= r_mag;
}

/* ------------------------------------------------------------------------- */
void
ik_quat_mul_no_normalize(ikreal_t q[4], const ikreal_t q2[4])
{
    ikreal_t q1[4];
    ik_quat_copy(q1, q);

#define w1 q1[3]
#define x1 q1[0]
#define y1 q1[1]
#define z1 q1[2]
#define w2 q2[3]
#define x2 q2[0]
#define y2 q2[1]
#define z2 q2[2]

    q[3] = w1*w2 - x1*x2 - y1*y2 - z1*z2;
    q[0] = w1*x2 + x1*w2 + y1*z2 - z1*y2;
    q[1] = w1*y2 + y1*w2 + z1*x2 - x1*z2;
    q[2] = w1*z2 + z1*w2 + x1*y2 - y1*x2;

#undef w1
#undef x1
#undef y1
#undef z1
#undef w2
#undef x2
#undef y2
#undef z2
}
void
ik_quat_mul_quat(ikreal_t q1[4], const ikreal_t q2[4])
{
    ik_quat_mul_no_normalize(q1, q2);
    ik_quat_normalize(q1);
}

/* ------------------------------------------------------------------------- */
void
ik_quat_nmul_no_normalize(ikreal_t q[4], const ikreal_t q2[4])
{
    ikreal_t q1[4];
    ik_quat_copy(q1, q);

#define w1 q1[3]
#define x1 q1[0]
#define y1 q1[1]
#define z1 q1[2]
#define w2 q2[3]
#define x2 q2[0]
#define y2 q2[1]
#define z2 q2[2]

    q[3] = w1*w2 + x1*x2 + y1*y2 + z1*z2;
    q[0] = -w1*x2 + x1*w2 - y1*z2 + z1*y2;
    q[1] = -w1*y2 + y1*w2 - z1*x2 + x1*z2;
    q[2] = -w1*z2 + z1*w2 - x1*y2 + y1*x2;

#undef w1
#undef x1
#undef y1
#undef z1
#undef w2
#undef x2
#undef y2
#undef z2
}
void
ik_quat_nmul_quat(ikreal_t q1[4], const ikreal_t q2[4])
{
    ik_quat_nmul_no_normalize(q1, q2);
    ik_quat_normalize(q1);
}

/* ------------------------------------------------------------------------- */
void
ik_quat_mul_scalar(ikreal_t q[4], ikreal_t scalar)
{
    q[0] *= scalar;
    q[1] *= scalar;
    q[2] *= scalar;
    q[3] *= scalar;
}

/* ------------------------------------------------------------------------- */
void
ik_quat_div_scalar(ikreal_t q[4], ikreal_t scalar)
{
    if (scalar == 0.0)
        ik_quat_set_identity(q);
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
ik_quat_dot(const ikreal_t q1[4], const ikreal_t q2[4])
{
    return q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2] + q1[3]*q2[3];
}

/* ------------------------------------------------------------------------- */
void
ik_quat_ensure_positive_sign(ikreal_t q1[4])
{
    ikreal_t unit[4] = {0, 0, 0, 1};
    ikreal_t dot = ik_quat_dot(q1, unit);
    if (dot < 0.0)
        ik_quat_negate(q1);
}

/* ------------------------------------------------------------------------- */
void
ik_quat_angle(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t cos_a, sin_a, angle, denominator;

    denominator = 1.0 / ik_vec3_length(v1) / ik_vec3_length(v2);
    cos_a = ik_vec3_dot(v1, v2) * denominator;
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        memcpy(q, v1, sizeof(ikreal_t) * 3);
        ik_vec3_cross(q, v2);
        ik_vec3_normalize(q);

        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(cos_a);
        cos_a = cos(angle * 0.5);
        sin_a = sin(angle * 0.5);
        ik_vec3_mul_scalar(q, sin_a);
        q[3] = cos_a; /* w component */
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        ik_quat_set_identity(q);
    }
}

/* ------------------------------------------------------------------------- */
void
ik_quat_angle_no_normalize(ikreal_t q[4], const ikreal_t v1[3], const ikreal_t v2[3])
{
    ikreal_t cos_a, sin_a, angle;

    cos_a = ik_vec3_dot(v1, v2);
    if (cos_a >= -1.0 && cos_a <= 1.0)
    {
        /* calculate axis of rotation and write it to the quaternion's vector section */
        ik_vec3_copy(q, v1);
        ik_vec3_cross(q, v2);
        /* would usually normalize here, but cross product of two normalized
         * vectors is already normalized*/

        /* quaternion's vector needs to be weighted with sin_a */
        angle = acos(cos_a);
        cos_a = cos(angle * 0.5);
        sin_a = sin(angle * 0.5);
        ik_vec3_mul_scalar(q, sin_a);
        q[3] = cos_a; /* w component */
    }
    else
    {
        /* Important! otherwise garbage happens when applying initial rotations */
        ik_quat_set_identity(q);
    }
}

/* ------------------------------------------------------------------------- */
void
ik_quat_print(char* buf, const ikreal_t q[4])
{

}
