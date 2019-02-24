#ifndef IK_ATTACHMENT_H
#define IK_ATTACHMENT_H

#include "ik/config.h"
#include "ik/refcount.h"

#define IK_ATTACHMENT_HEAD \
    IK_REFCOUNT_HEAD

#define IK_ATTACHMENT_LIST \
    X(CONSTRAINT, constraint) \
    X(EFFECTOR, effector) \
    X(POLE, pole) \
    X(SOLVER, solver)

C_BEGIN

enum ik_attachment_type_e
{
#define X(upper, lower) IK_ATTACHMENT_##upper,
    IK_ATTACHMENT_LIST
#undef X

    IK_ATTACHMENT_COUNT
};

struct ik_attachment_t
{
    IK_ATTACHMENT_HEAD
};

IK_PRIVATE_API const char*
ik_attachment_to_str(enum ik_attachment_type_e type);

C_END

#endif /* IK_ATTACHMENT_H */
