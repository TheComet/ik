#include "cstructures/memory.h"
#include "ik/chain_tree.h"
#include "ik/log.h"
#include "ik/node.h"
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
    vector_init(&chain->nodes, sizeof(struct ik_node*));
    vector_init(&chain->children, sizeof(struct ik_chain));
    vector_init(&chain->dead_nodes, sizeof(struct ik_node*));
}

/* ------------------------------------------------------------------------- */
void
chain_tree_deinit(struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        chain_tree_deinit(child_chain);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_NODE(chain, node)
        IK_DECREF(node);
    CHAIN_END_EACH
    CHAIN_FOR_EACH_DEAD_NODE(chain, node)
        IK_DECREF(node);
    CHAIN_END_EACH

    vector_deinit(&chain->children);
    vector_deinit(&chain->nodes);
    vector_deinit(&chain->dead_nodes);
}

void
chain_tree_clear(struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        chain_tree_clear(child_chain);
    CHAIN_END_EACH

    /*
     * Decref all nodes INCLUDING the base node, because there are no parent
     * chains
     */
    CHAIN_FOR_EACH_NODE(chain, node)
        IK_DECREF(node);
    CHAIN_END_EACH
    CHAIN_FOR_EACH_DEAD_NODE(chain, node)
        IK_DECREF(node);
    CHAIN_END_EACH

    vector_clear_compact(&chain->children);
    vector_clear_compact(&chain->nodes);
    vector_clear_compact(&chain->dead_nodes);
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
chain_add_node(struct ik_chain* chain, const struct ik_node* node)
{
    if (vector_push(&chain->nodes, &node) == 0)
    {
        IK_INCREF(node);
        return 0;
    }

    return -1;
}

/* ------------------------------------------------------------------------- */
int
chain_add_dead_node(struct ik_chain* chain, const struct ik_node* node)
{
    if (vector_push(&chain->dead_nodes, &node) == 0)
    {
        IK_INCREF(node);
        return 0;
    }

    return -1;
}

/* ------------------------------------------------------------------------- */
static int
chain_tree_build_recursive(struct ik_chain* chain,
                           const struct ik_node* node,
                           const struct ik_subtree* subtree)
{
    switch (subtree_check_children_up_to(subtree, node, 2))
    {
        case 2: {
            NODE_FOR_EACH(node, user_data, child_node)
                struct ik_chain* child_chain;

                if (!subtree_check_node(subtree, child_node))
                {
                    if (chain_add_dead_node(chain, child_node) != 0)
                        return -1;
                    continue;
                }

                child_chain = chain_create_child(chain);
                if (child_chain == NULL)
                    return -1;

                if (chain_tree_build_recursive(child_chain, child_node, subtree) != 0)
                    return -1;
                if (chain_add_node(child_chain, node) != 0)
                    return -1;
            NODE_END_EACH
        } break;

        case 1: {
            NODE_FOR_EACH(node, user_data, child_node)
                if (!subtree_check_node(subtree, child_node))
                    continue;
                if (chain_tree_build_recursive(chain, child_node, subtree) != 0)
                    return -1;
            NODE_END_EACH
        } break;

        case 0: {
            /* NOTE: This causes dead nodes to be added to chains that don't
             * have child chains. Doesn't make sense anymore
            NODE_FOR_EACH(node, user_data, child_node)
                if (chain_add_dead_node(chain, child_node) != 0)
                    return -1;
            NODE_END_EACH*/
        } break;
    }

    if (chain_add_node(chain, node) != 0)
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
