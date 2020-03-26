#include "cstructures/memory.h"
#include "ik/chain.h"
#include "ik/log.h"
#include "ik/node.h"
#include "ik/subtree.h"
#include <assert.h>
#include <stdio.h>

/* ------------------------------------------------------------------------- */
struct ik_chain*
chain_create(void)
{
    struct ik_chain* chain = MALLOC(sizeof *chain);
    if (chain == NULL)
    {
        ik_log_out_of_memory("chain_create()");
        return NULL;
    }

    if (chain_init(chain) != 0)
    {
        FREE(chain);
        return NULL;
    }

    return chain;
}

/* ------------------------------------------------------------------------- */
void
chain_destroy(struct ik_chain* chain)
{
    chain_deinit(chain);
    FREE(chain);
}

/* ------------------------------------------------------------------------- */
int
chain_init(struct ik_chain* chain)
{
    if (vector_init(&chain->nodes, sizeof(struct ik_node*)) == 0)
        return -1;

    if (vector_init(&chain->children, sizeof(struct ik_chain)) == 0)
    {
        vector_deinit(&chain->nodes);
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
void
chain_deinit(struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        chain_deinit(child_chain);
    CHAIN_END_EACH

    CHAIN_FOR_EACH_NODE(chain, node)
        IK_DECREF(node);
    CHAIN_END_EACH

    vector_deinit(&chain->children);
    vector_deinit(&chain->nodes);
}

/* ------------------------------------------------------------------------- */
void
chain_clear_recursive(struct ik_chain* chain)
{
    vec_size_t node_idx;

    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        chain_clear_recursive(child_chain);
    CHAIN_END_EACH

    /*
     * Decref all nodes except for the base node, because the base node is
     * stored as part of the parent chain's leaf node
     */
    for (node_idx = 0; node_idx < chain_length(chain) - 1; ++node_idx)
    {
        struct ik_node* node  = chain_get_node(chain, node_idx);
        IK_DECREF(node);
    }

    vector_clear_compact(&chain->children);
    vector_clear_compact(&chain->nodes);
}
void
chain_clear(struct ik_chain* chain)
{
    CHAIN_FOR_EACH_CHILD(chain, child_chain)
        chain_clear_recursive(child_chain);
    CHAIN_END_EACH

    /*
     * Decref all nodes INCLUDING the base node, because there are no parent
     * chains
     */
    CHAIN_FOR_EACH_NODE(chain, node)
        IK_DECREF(node);
    CHAIN_END_EACH

    vector_clear_compact(&chain->children);
    vector_clear_compact(&chain->nodes);
}

/* ------------------------------------------------------------------------- */
struct ik_chain*
chain_create_child(struct ik_chain* chain)
{
    return vector_emplace(&chain->children);
}

/* ------------------------------------------------------------------------- */
int
chain_add_node(struct ik_chain* chain, const struct ik_node* node)
{
    if (vector_push(&chain->nodes, &node) == VECTOR_OK)
    {
        IK_INCREF(node);
        return 0;
    }

    return -1;
}

/* ------------------------------------------------------------------------- */
static int
chain_tree_rebuild_recursive(struct ik_chain* chain,
                             const struct ik_node* node,
                             const struct ik_subtree* subtree)
{
    return 0;
}
int
chain_tree_rebuild(struct ik_chain* chain, const struct ik_subtree* subtree)
{
    return chain_tree_rebuild_recursive(chain, subtree->root, subtree);
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

/* ------------------------------------------------------------------------- */
#ifdef IK_DOT_OUTPUT
static void
dump_chain(const chain_t* chain, FILE* fp)
{
    int last_idx = chain_length(chain) - 1;
    if (last_idx > 0)
    {
        fprintf(fp, "    %d [shape=record];\n", chain_get_node(chain, 0)->guid);
        fprintf(fp, "    %d [shape=record];\n", chain_get_base_node(chain)->guid);
    }

    while (last_idx-- > 0)
    {
        fprintf(fp, "    %d -- %d [color=\"1.0 0.5 1.0\"];\n",
            chain_get_node(chain, last_idx + 0)->guid,
            chain_get_node(chain, last_idx + 1)->guid);
    }

    VECTOR_FOR_EACH(&chain->children, chain_t, child)
        dump_chain(child, fp);
    VECTOR_END_EACH
}
static void
dump_node(const ik_node* node, FILE* fp)
{
    if (node->effector != NULL)
        fprintf(fp, "    %d [color=\"0.6 0.5 1.0\"];\n", node->guid);
    NODE_FOR_EACH(node, guid, child)
        fprintf(fp, "    %d -- %d;\n", node->guid, guid);
        dump_node(child, fp);
    NODE_END_EACH
}
void
dump_to_dot(const ik_node* node, const vector_t* chains, const char* file_name)
{
    FILE* fp = fopen(file_name, "w");
    if (fp == NULL)
        return;

    fprintf(fp, "graph chain_tree {\n");
    dump_node(node, fp);
    VECTOR_FOR_EACH(chains, chain_t, chain)
        dump_chain(chain, fp);
    VECTOR_END_EACH
    fprintf(fp, "}\n");

    fclose(fp);
}
#endif
