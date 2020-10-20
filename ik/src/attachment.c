#include "ik/attachment.h"
#include "ik/algorithm.h"
#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/pole.h"
#include "ik/tree_object.h"
#include <stddef.h>

/* ------------------------------------------------------------------------- */
struct ik_attachment*
ik_attachment_alloc(uintptr_t obj_size, ik_deinit_func deinit)
{
    struct ik_attachment* att = (struct ik_attachment*)
        ik_refcounted_alloc(obj_size, deinit);
    if (att == NULL)
        return NULL;

    return att;
}

/* ------------------------------------------------------------------------- */
int
ik_attachment_duplicate_from_tree(struct ik_tree_object* dst, const struct ik_tree_object* src)
{
#define X1(upper, lower, arg) X(upper, lower)
#define X(upper, lower)                                                       \
    if (ik_##lower##_duplicate_from_tree(dst, src) != 0)                      \
        return -1;
    IK_ATTACHMENT_LIST
#undef X
#undef X1
    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_attachment_reference_from_tree(struct ik_tree_object* dst, const struct ik_tree_object* src)
{
#define X1(upper, lower, arg) X(upper, lower)
#define X(upper, lower)                                                       \
        if (src->lower)                                                       \
            ik_tree_object_attach_##lower(dst, src->lower);
    IK_ATTACHMENT_LIST
#undef X
#undef X1
}
