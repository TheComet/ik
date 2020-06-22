#include "ik/subtree.h"
#include "ik/node.h"
#include "ik/log.h"
#include <stddef.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
int
subtree_init(struct ik_subtree* st)
{
    st->root = NULL;
    if (vector_init(&st->leaves, sizeof(struct ik_node*)) != 0)
    {
        ik_log_out_of_memory("ik_subtree_init()");
        return -1;
    }
    return 0;
}

/* ------------------------------------------------------------------------- */
void
subtree_deinit(struct ik_subtree* st)
{
    vector_deinit(&st->leaves);
}

/* ------------------------------------------------------------------------- */
void
subtree_set_root(struct ik_subtree* st, const struct ik_node* root)
{
    st->root = root;
}

/* ------------------------------------------------------------------------- */
int
subtree_add_leaf(struct ik_subtree* st, const struct ik_node* leaf)
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
subtree_is_leaf_node(const struct ik_subtree* st, const struct ik_node* node)
{
    VECTOR_FOR_EACH(&st->leaves, struct ik_node*, p_node)
        if (node == *p_node)
            return 1;
    VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
subtree_check_node(const struct ik_subtree* st, const struct ik_node* node)
{
    VECTOR_FOR_EACH(&st->leaves, struct ik_node*, p_node)
        const struct ik_node* cur_node = *p_node;
        for (; cur_node != st->root; cur_node = cur_node->parent)
        {
            if (cur_node == node)
                return 1;
        }
        if (st->root == node)
            return 1;
    VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
int
subtree_check_children_up_to(const struct ik_subtree* st,
                             const struct ik_node* node,
                             int max_count)
{
    int found = 0;
    NODE_FOR_EACH(node, user_data, child)
        found += subtree_check_node(st, child);
        if (found == max_count)
            break;
    NODE_END_EACH

    return found;
}
