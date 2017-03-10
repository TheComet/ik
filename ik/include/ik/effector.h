#ifndef EFFECTOR_H
#define EFFECTOR_H

#include "ik/gen/config.h"
#include "ik/pstdint.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_HEADER_BEGIN

struct ik_node_t;

struct ik_effector_t
{
    struct vec3_t target_position;
    struct quat_t target_rotation;
    uint16_t chain_length;
};

IK_PUBLIC_API struct ik_effector_t*
ik_effector_create(void);

IK_PUBLIC_API void
ik_effector_construct(struct ik_effector_t* effector);

IK_PUBLIC_API void
ik_effector_destroy(struct ik_effector_t* effector);

C_HEADER_END

#endif /* EFFECTOR_H */
