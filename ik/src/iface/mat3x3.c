#include "ik/impl/mat3x3.h"

/* ------------------------------------------------------------------------- */
void
ik_mat3x3_from_quat(ikreal_t m[9], const ikreal_t q[4])
{
    ikreal_t xx = q[0]*q[0];
    ikreal_t xy = q[0]*q[1];
    ikreal_t xz = q[0]*q[2];
    ikreal_t xw = q[0]*q[3];
    ikreal_t yy = q[1]*q[1];
    ikreal_t yz = q[1]*q[2];
    ikreal_t yw = q[1]*q[3];
    ikreal_t zz = q[2]*q[2];
    ikreal_t zw = q[2]*q[3];

    m[0] = 1.0 - 2.0 * (yy + zz);
    m[1] =       2.0 * (xy - zw);
    m[2] =       2.0 * (xz + yw);
    m[3] =       2.0 * (xy + zw);
    m[4] = 1.0 - 2.0 * (xx + zz);
    m[5] =       2.0 * (yz - xw);
    m[6] =       2.0 * (xz - yw);
    m[7] =       2.0 * (yz + xw);
    m[8] = 1.0 - 2.0 * (xx + yy);
}
