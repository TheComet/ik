#ifndef QUATERNION_H
#define QUATERNION_H

#include "ik/gen/config.h"
#include "ik/vec3.h"

C_HEADER_BEGIN

typedef union quat_t
{
    struct {
        vec3_t v;
        ik_real w;
    } vw;
    struct {
        ik_real x;
        ik_real y;
        ik_real z;
        ik_real w;
    } q;
    ik_real f[4];
} quat_t;

IK_PUBLIC_API void
quat_set_identity(ik_real* q);

IK_PUBLIC_API ik_real
quat_mag(const ik_real* q);

IK_PUBLIC_API void
quat_normalise(ik_real* q);

IK_PUBLIC_API void
quat_mul(ik_real* q1, const ik_real* q2);

C_HEADER_END

#endif /* QUATERNION_H */
