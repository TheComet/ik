#pragma once

#include "ik/config.h"
#include "ik/refcount.h"

#define IK_ATTACHMENT_HEAD                   \
    IK_REFCOUNTED_HEAD                       \
    struct ik_tree_object* tree_object;

#define IK_ATTACHMENT_LIST                   \
    X1(ALGORITHM, algorithm, const char*)    \
    X(CONSTRAINT, constraint)                \
    X(EFFECTOR,   effector)                  \
    X(POLE,       pole)

C_BEGIN

enum ik_attachment_type
{
#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower) IK_ATTACHMENT_##upper,
    IK_ATTACHMENT_LIST
#undef X
#undef X1

    IK_ATTACHMENT_COUNT
};

struct ik_attachment
{
    IK_ATTACHMENT_HEAD
};

IK_PRIVATE_API struct ik_attachment*
ik_attachment_alloc(uintptr_t obj_size, ik_deinit_func deinit);

IK_PRIVATE_API void
ik_attachment_init(struct ik_attachment* att);

IK_PUBLIC_API int
ik_attachment_duplicate_all(struct ik_tree_object* dst, const struct ik_tree_object* src);

C_END
