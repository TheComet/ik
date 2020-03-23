#ifndef IK_ATTACHMENT_H
#define IK_ATTACHMENT_H

#include "ik/config.h"
#include "ik/refcount.h"

#define IK_ATTACHMENT_HEAD \
    IK_REFCOUNT_HEAD       \
    struct ik_node* node;

#define IK_ATTACHMENT_LIST    \
    X(ALGORITHM,  algorithm)  \
    X(CONSTRAINT, constraint) \
    X(EFFECTOR,   effector)   \
    X(POLE,       pole)       \
    /*X(SOLVER,     solver)*/

C_BEGIN

enum ik_attachment_type
{
#define X(upper, lower) IK_ATTACHMENT_##upper,
    IK_ATTACHMENT_LIST
#undef X

    IK_ATTACHMENT_COUNT
};

struct ik_attachment
{
    IK_ATTACHMENT_HEAD
};

IK_PRIVATE_API struct ik_attachment*
ik_attachment_alloc(uintptr_t obj_size, ik_deinit_func deinit);

C_END

#endif /* IK_ATTACHMENT_H */
