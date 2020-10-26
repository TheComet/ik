#include "ik/subtree.h"
#include "ik/bone.h"
#include "ik/log.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
void
subtree_init(struct ik_subtree* st)
{
    st->root = NULL;
    vector_init(&st->leaves, sizeof(struct ik_bone*));
}

/* ------------------------------------------------------------------------- */
void
subtree_deinit(struct ik_subtree* st)
{
    vector_deinit(&st->leaves);
}

/* ------------------------------------------------------------------------- */
void
subtree_set_root(struct ik_subtree* st, struct ik_bone* root)
{
    st->root = root;
}

/* ------------------------------------------------------------------------- */
int
subtree_add_leaf(struct ik_subtree* st, struct ik_bone* leaf)
{
    if (vector_push(&st->leaves, &leaf) != 0)
    {
        ik_log_out_of_memory("ik_subtree_add_leaf()");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
int
subtree_is_leaf_bone(const struct ik_subtree* st, const struct ik_bone* bone)
{
    VECTOR_FOR_EACH(&st->leaves, struct ik_bone*, p_bone)
        if (bone == *p_bone)
            return 1;
    VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
subtree_check_bone(const struct ik_subtree* st, const struct ik_bone* bone)
{
    VECTOR_FOR_EACH(&st->leaves, struct ik_bone*, p_bone)
        const struct ik_bone* cur_bone = *p_bone;
        for (; cur_bone != st->root; cur_bone = (struct ik_bone*)cur_bone->parent)
        {
            if (cur_bone == bone)
                return 1;
        }
        if (st->root == bone)
            return 1;
    VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
subtree_check_children_up_to(const struct ik_subtree* st,
                             const struct ik_bone* bone,
                             int max_count)
{
    int found = 0;
    BONE_FOR_EACH_CHILD(bone, child)
        found += subtree_check_bone(st, child);
        if (found == max_count)
            break;
    BONE_END_EACH

    return found;
}
