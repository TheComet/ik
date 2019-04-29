#ifndef IK_ATTACHMENT_H
#define IK_ATTACHMENT_H

#include "ik/config.h"
#include "ik/refcount.h"

#define IK_ATTACHMENT_HEAD \
    IK_REFCOUNT_HEAD

#define IK_ATTACHMENT_LIST \
    /*X(CONSTRAINT, constraint) */\
    X(EFFECTOR,   effector,  struct ik_effector_t) \
    /*X(POLE,       pole) \
    X(SOLVER,     solver)*/

C_BEGIN

enum ik_attachment_type_e
{
#define X(upper, lower, type) IK_ATTACHMENT_##upper,
    IK_ATTACHMENT_LIST
#undef X

    IK_ATTACHMENT_COUNT
};

struct ik_attachment_t
{
    IK_ATTACHMENT_HEAD
};

C_END

#endif /* IK_ATTACHMENT_H */
