#include "ik/subtree.h"
#include <stddef.h>

/* ------------------------------------------------------------------------- */
int
ik_subtree_init(struct ik_subtree* st)
{
    st->root = NULL;
    if (vector_init(&st->leaves, sizeof(struct ik_node*)) != VECTOR_OK)
        return -1;
    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_subtree_deinit(struct ik_subtree* st)
{
    vector_deinit(&st->leaves);
}
