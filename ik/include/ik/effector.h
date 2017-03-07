#ifndef EFFECTOR_H
#define EFFECTOR_H

#include "ik/config.h"
#include "ik/pstdint.h"
#include "ik/vector3.h"
#include "ik/quaternion.h"

C_HEADER_BEGIN

struct node_t;

struct effector_t
{
    struct vector3_t target_position;
    struct quaternion_t target_rotation;
    uint16_t chain_length;
};

IK_PUBLIC_API struct effector_t*
effector_create(void);

IK_PUBLIC_API void
effector_construct(struct effector_t* effector);

IK_PUBLIC_API void
effector_destroy(struct effector_t* effector);

C_HEADER_END

#endif /* EFFECTOR_H */
