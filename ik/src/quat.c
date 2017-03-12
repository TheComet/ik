#include "ik/quat.h"
#include <math.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
void
quat_set_identity(ik_real* q)
{
    memset(q, 0, sizeof(ik_real) * 3);
    q[3] = 1;
}

/* ------------------------------------------------------------------------- */
ik_real
quat_mag(const ik_real* q)
{
    return sqrt(q[3]*q[3] + q[2]*q[2] + q[1]*q[1] + q[0]*q[0]);
}

/* ------------------------------------------------------------------------- */
void
quat_normalise(ik_real* q)
{
    ik_real mag = quat_mag(q);
    if(mag != 0.0)
        mag = 1.0 / mag;
    q[0] *= mag;
    q[1] *= mag;
    q[2] *= mag;
    q[3] *= mag;
}

/* ------------------------------------------------------------------------- */
void
quat_mul(ik_real* q1, const ik_real* q2)
{
    ik_real v1[3];
    ik_real v2[3];
    memcpy(v1, q1, sizeof(ik_real) * 3);
    memcpy(v2, q2, sizeof(ik_real) * 3);

    vec3_mul_scalar(v1, q2[3]);
    vec3_mul_scalar(v2, q1[3]);
    q1[3] = q1[3]*q2[3] - vec3_dot(q1, q2);
    vec3_cross(q1, q2);
    vec3_add_vec3(q1, v1);
    vec3_add_vec3(q1, v2);

    quat_normalise(q1);
}
