#include "cstructures/memory.h"
#include "ik/chain_tree.h"
#include "ik/log.h"
#include "ik/bone.h"
#include "ik/subtree.h"
#include <assert.h>
#include <stdio.h>

/* ------------------------------------------------------------------------- */
struct ik_chain*
chain_tree_create(void)
{
    struct ik_chain* chain = MALLOC(sizeof *chain);
    if (chain == NULL)
    {
        ik_log_out_of_memory("chain_create()");
        return NULL;
    }

    chain_tree_init(chain);

    return chain;
}

/* ------------------------------------------------------------------------- */
void
chain_tree_destroy(struct ik_chain* chain)
{
    chain_tree_deinit(chain);
    FREE(chain);
}

/* ------------------------------------------------------------------------- */
void
chain_tree_init(struct ik_chain* chain)
{
    vector_init(&chain->bones, sizeof(struct ik_bone*));
    vector_init(&chain->children, sizeof(struct ik_chain));
}

/* ------------------------------------------------------------------------- */
void
chain_tree_deinit(struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        chain_tree_deinit(child_chain);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_BONE(chain, bone)
        IK_DECREF(bone);
    CHAIN_END_EACH

    vector_deinit(&chain->children);
    vector_deinit(&chain->bones);
}

void
chain_tree_clear(struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        chain_tree_clear(child_chain);
    CHAIN_END_EACH

    /*
     * Decref all bones INCLUDING the base bone, because there are no parent
     * chains
     */
    CHAIN_FOR_EACH_BONE(chain, bone)
        IK_DECREF(bone);
    CHAIN_END_EACH

    vector_clear_compact(&chain->children);
    vector_clear_compact(&chain->bones);
}

/* ------------------------------------------------------------------------- */
struct ik_chain*
chain_create_child(struct ik_chain* chain)
{
    struct ik_chain* child = vector_emplace(&chain->children);
    if (child == NULL)
        return NULL;

    chain_tree_init(child);

    return child;
}

/* ------------------------------------------------------------------------- */
int
chain_add_bone(struct ik_chain* chain, const struct ik_bone* bone)
{
    if (vector_push(&chain->bones, &bone) == 0)
    {
        IK_INCREF(bone);
        return 0;
    }

    return -1;
}

/* ------------------------------------------------------------------------- */
static int
chain_tree_build_recursive(struct ik_chain* chain,
                           const struct ik_bone* bone,
                           const struct ik_subtree* subtree)
{
    switch (subtree_check_children_up_to(subtree, bone, 2))
    {
        case 2: {
            BONE_FOR_EACH_CHILD(bone, child_bone)
                struct ik_chain* child_chain;

                if (!subtree_check_bone(subtree, child_bone))
                    continue;

                child_chain = chain_create_child(chain);
                if (child_chain == NULL)
                    return -1;

                if (chain_tree_build_recursive(child_chain, child_bone, subtree) != 0)
                    return -1;
                if (chain_add_bone(child_chain, bone) != 0)
                    return -1;
            BONE_END_EACH
        } break;

        case 1: {
            BONE_FOR_EACH_CHILD(bone, child_bone)
                if (!subtree_check_bone(subtree, child_bone))
                    continue;
                if (chain_tree_build_recursive(chain, child_bone, subtree) != 0)
                    return -1;
            BONE_END_EACH
        } break;

        case 0: {
            /* NOTE: This causes dead bones to be added to chains that don't
             * have child chains. Doesn't make sense anymore
            BONE_FOR_EACH_CHILD(bone, user_data, child_bone)
                if (chain_add_dead_bone(chain, child_bone) != 0)
                    return -1;
            BONE_END_EACH*/
        } break;
    }

    if (chain_add_bone(chain, bone) != 0)
        return -1;

    return 0;
}
int
chain_tree_build(struct ik_chain* chain, const struct ik_subtree* subtree)
{
    return chain_tree_build_recursive(chain, subtree->root, subtree);
}

/* ------------------------------------------------------------------------- */
int
count_chains(const struct ik_chain* chain)
{
    int counter = 1;
    CHAIN_FOR_EACH_CHILD(chain, child)
        counter += count_chains(child);
    CHAIN_END_EACH
    return counter;
}
