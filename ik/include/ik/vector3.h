#ifndef VECTOR3_H
#define VECTOR3_H

#include "ik/config.h"

C_HEADER_BEGIN

struct vector3_t
{
    ik_real x;
    ik_real y;
    ik_real z;
};

IK_PUBLIC_API void
vector3_set_zero(struct vector3_t* v);

IK_PUBLIC_API ik_real
vector3_length_squared(const struct vector3_t* v);

IK_PUBLIC_API ik_real
vector3_length(const struct vector3_t* v);

IK_PUBLIC_API void
vector3_normalise(struct vector3_t* v);

IK_PUBLIC_API ik_real
vector3_dot(const struct vector3_t* v1, const struct vector3_t* v2);

C_HEADER_END

#endif /* VECTOR3_H */
