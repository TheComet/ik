#pragma once

#include "ik/mat3x4.h"
#include <string.h>

C_BEGIN

static inline void
ik_mat3x4_set_identity(ikreal m[12])
{
    m[0] = 1.0;   m[3] = 0.0;   m[6] = 0.0;   m[9]  = 0.0;
    m[1] = 0.0;   m[4] = 1.0;   m[7] = 0.0;   m[10] = 0.0;
    m[2] = 0.0;   m[5] = 0.0;   m[8] = 1.0;   m[11] = 0.0;
}

static inline void
ik_mat3x4_copy(ikreal m[12], const ikreal src[12])
{
    memcpy(m, src, sizeof(ikreal) * 12);
}

static inline void
ik_mat3x4_set_pos(ikreal m[12], const ikreal v[3])
{
    m[0] = 1.0;   m[3] = 0.0;   m[6] = 0.0;   m[9]  = v[0];
    m[1] = 0.0;   m[4] = 1.0;   m[7] = 0.0;   m[10] = v[1];
    m[2] = 0.0;   m[5] = 0.0;   m[8] = 1.0;   m[11] = v[2];
}

static inline void
ik_mat3x4_set_rot(ikreal m[12], const ikreal q[4])
{
    m[0] = 1.0f - 2.0f * q[1] * q[1] - 2.0f * q[2] * q[2];
    m[1] = 2.0f * q[0] * q[1] + 2.0f * q[3] * q[2];
    m[2] = 2.0f * q[0] * q[2] - 2.0f * q[3] * q[1];
    m[3] = 2.0f * q[0] * q[1] - 2.0f * q[3] * q[2];
    m[4] = 1.0f - 2.0f * q[0] * q[0] - 2.0f * q[2] * q[2];
    m[5] = 2.0f * q[1] * q[2] + 2.0f * q[3] * q[0];
    m[6] = 2.0f * q[0] * q[2] + 2.0f * q[3] * q[1];
    m[7] = 2.0f * q[1] * q[2] - 2.0f * q[3] * q[0];
    m[8] = 1.0f - 2.0f * q[0] * q[0] - 2.0f * q[1] * q[1];
}

static inline void
ik_mat3x4_set_pos_rot(ikreal m[12], const ikreal v[3], const ikreal q[4])
{
    m[0] = 1.0f - 2.0f * q[1] * q[1] - 2.0f * q[2] * q[2];
    m[1] = 2.0f * q[0] * q[1] + 2.0f * q[3] * q[2];
    m[2] = 2.0f * q[0] * q[2] - 2.0f * q[3] * q[1];
    m[3] = 2.0f * q[0] * q[1] - 2.0f * q[3] * q[2];
    m[4] = 1.0f - 2.0f * q[0] * q[0] - 2.0f * q[2] * q[2];
    m[5] = 2.0f * q[1] * q[2] + 2.0f * q[3] * q[0];
    m[6] = 2.0f * q[0] * q[2] + 2.0f * q[3] * q[1];
    m[7] = 2.0f * q[1] * q[2] - 2.0f * q[3] * q[0];
    m[8] = 1.0f - 2.0f * q[0] * q[0] - 2.0f * q[1] * q[1];

    m[9]  = v[0];
    m[10] = v[1];
    m[11] = v[2];
}

static inline void
ik_mat3x4_set_basis_vectors(ikreal m[12], const ikreal ex[3], const ikreal ey[3], const ikreal ez[3], const ikreal et[3])
{
    memcpy(&m[0], ex, sizeof(ikreal) * 3);
    memcpy(&m[3], ey, sizeof(ikreal) * 3);
    memcpy(&m[6], ez, sizeof(ikreal) * 3);
    memcpy(&m[9], et, sizeof(ikreal) * 3);
}

static inline void
ik_mat3x4_set(ikreal m[12],
              ikreal m00, ikreal m01, ikreal m02, ikreal m03,
              ikreal m10, ikreal m11, ikreal m12, ikreal m13,
              ikreal m20, ikreal m21, ikreal m22, ikreal m23)
{
    m[0] = m00;   m[3] = m01;   m[6] = m02;   m[9]  = m03;
    m[1] = m10;   m[4] = m11;   m[7] = m12;   m[10] = m13;
    m[2] = m20;   m[5] = m21;   m[8] = m22;   m[11] = m23;
}

static inline void
ik_mat3x4_mul_mat3x4(ikreal m[12], const ikreal m2[12])
{
    ikreal m1[12];
    ik_mat3x4_copy(m1, m);

    /* First column */
    m[0] = m1[0]*m2[0] + m1[3]*m2[1] + m1[6]*m2[2] /* + m1[9]*0 */;
    m[1] = m1[1]*m2[0] + m1[4]*m2[1] + m1[7]*m2[2] /* + m1[10]*0 */;
    m[2] = m1[2]*m2[0] + m1[5]*m2[1] + m1[8]*m2[2] /* + m1[11]*0 */;

    /* Second column */
    m[3] = m1[0]*m2[3] + m1[3]*m2[4] + m1[6]*m2[5] /* + m1[9]*0 */;
    m[4] = m1[1]*m2[3] + m1[4]*m2[4] + m1[7]*m2[5] /* + m1[10]*0 */;
    m[5] = m1[2]*m2[3] + m1[5]*m2[4] + m1[8]*m2[5] /* + m1[11]*0 */;

    /* Third column */
    m[6] = m1[0]*m2[6] + m1[3]*m2[7] + m1[6]*m2[8] /* + m1[9]*0 */;
    m[7] = m1[1]*m2[6] + m1[4]*m2[7] + m1[7]*m2[8] /* + m1[10]*0 */;
    m[8] = m1[2]*m2[6] + m1[5]*m2[7] + m1[8]*m2[8] /* + m1[11]*0 */;

    /* Fourth column */
    m[9]  = m1[0]*m2[9] + m1[3]*m2[10] + m1[6]*m2[11] + m1[9];
    m[10] = m1[1]*m2[9] + m1[4]*m2[10] + m1[7]*m2[11] + m1[10];
    m[11] = m1[2]*m2[9] + m1[5]*m2[10] + m1[8]*m2[11] + m1[11];
}

static inline void
ik_mat3x4_mul_vec3(const ikreal m[12], ikreal v[3])
{
    ikreal vx   = m[0]*v[0] + m[3]*v[1] + m[6]*v[2] + m[9];
    ikreal vy   = m[1]*v[0] + m[4]*v[1] + m[7]*v[2] + m[10];
           v[2] = m[2]*v[0] + m[5]*v[1] + m[8]*v[2] + m[11];
    v[0] = vx;
    v[1] = vy;
}

static inline void
ik_mat3x4_mul_scalar(ikreal m[12], ikreal s)
{
    m[0] *= s;
    m[1] *= s;
    m[2] *= s;
    m[3] *= s;
    m[4] *= s;
    m[5] *= s;
    m[6] *= s;
    m[7] *= s;
    m[8] *= s;
    m[9] *= s;
    m[10] *= s;
    m[11] *= s;
}

C_END
