#include "ik/chain.h"
#include "ik/effector.h"
#include "ik/node.h"
#include "ik/util.h"
#include <assert.h>

struct effector_data_t
{

    ik_real rotation_weight;
    ik_real rotation_weight_decay;
};

/* ------------------------------------------------------------------------- */
static struct effector_data_t
calculate_rotation_weight_decays_recursive(struct ik_chain_t* chain)
{
    int average_count;
    int node_idx, node_count;
    struct effector_data_t effector_data;

    /*
     * Find the rotation weight of this chain's last node by averaging the
     * rotation weight of all child chain's first nodes.
     *
     * If there are no child chains, then the first node in the chain must
     * contain an effector. Initialise the weight parameters from said
     * effector.
     *
     * If there are child chains then average the effector data we've
     * accumulated.
     */
    average_count = 0;
    ORDERED_VECTOR_FOR_EACH(&chain->children, struct ik_chain_t, child)
        struct effector_data_t child_eff_data =
                calculate_rotation_weight_decays_recursive(child);
        effector_data.rotation_weight += child_eff_data.rotation_weight;
        effector_data.rotation_weight_decay += child_eff_data.rotation_weight_decay;
        ++average_count;
    ORDERED_VECTOR_END_EACH

    if(average_count == 0)
    {
        struct ik_node_t* effector_node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, 0);
        struct ik_effector_t* effector = effector_node->effector;

        effector_data.rotation_weight = effector->rotation_weight;
        effector_data.rotation_weight_decay = effector->rotation_decay;
    }
    else
    {
        ik_real div = 1.0 / average_count;
        effector_data.rotation_weight *= div;
        effector_data.rotation_weight_decay *= div;
    }

    /*
     * With the rotation weight of the last node calculated, we can now iterate
     * the nodes in the chain and set each rotation weight, decaying a little
     * bit every time. Note that we exclude the last node, because it will
     * be handled by the parent chain. If there is no parent chain then the
     * non-recursive caller of this function will set the rotation weight of
     * the root node.
     */
    node_count = ordered_vector_count(&chain->nodes) - 1;
    for (node_idx = 0; node_idx < node_count; ++node_idx)
    {
        struct ik_node_t* node = *(struct ik_node_t**)ordered_vector_get_element(&chain->nodes, node_idx);
        node->rotation_weight = effector_data.rotation_weight;
        effector_data.rotation_weight *= effector_data.rotation_weight_decay;
    }

    /* Rotation weight is now equal to that of this chain's base node */
    return effector_data;
}

/* ------------------------------------------------------------------------- */
void
ik_calculate_rotation_weight_decays(struct ik_chain_t* root_chain)
{
    /*
     * The root chain of the solver doesn't actually contain any nodes. It
     * merely acts as a convenient container for all of the disjoint chain
     * trees (in most cases there will be exactly one of those, i.e. there is
     * one continuous tree).
     *
     * The recursive version of this function does not set the rotation weight
     * of the first node in every tree that gets passed to it, but it does
     * return the rotation weight that *would have been set* (which gets used
     * recursively to calculate the average rotation weight in the case of
     * multiple child chains).
     *
     * For these reasons we iterate the chains and set the first node in each
     * chain to the returned rotation weight.
     */
    ORDERED_VECTOR_FOR_EACH(&root_chain->children, struct ik_chain_t, child)
        struct effector_data_t effector_data = calculate_rotation_weight_decays_recursive(root_chain);
        int last_idx = ordered_vector_count(&child->nodes) - 1;
        assert(last_idx > 0);
        struct ik_node_t* root_node = *(struct ik_node_t**)ordered_vector_get_element(&child->nodes, last_idx);
        root_node->rotation_weight = effector_data.rotation_weight;
    ORDERED_VECTOR_END_EACH
}
