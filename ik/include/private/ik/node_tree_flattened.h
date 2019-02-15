#ifndef IK_NODE_TREE_FLATTENED
#define IK_NODE_TREE_FLATTENED

#include "ik/config.h"
#include "ik/refcounted.h"

C_BEGIN

struct ik_node_data_t;
struct ik_node_t;

struct ik_ntf_t
{
    /*
     * Contiguous array of all node_data_t. All node_data refcounts point to
     * the first node's refounct, because in a sense, all nodes own each
     * other's memory (i.e. all node_data instances must be deleted before the
     * congiguous block of memory can be freed).
     */
    struct ik_node_data_t* nodes;

    struct {
        /* In-order indices */
        int lnr;
        int lnr_nodes_left_in_chain;
        /* Pre-order indices */
        int nlr;
        int nlr_nodes_left_in_chain;
    }* indices;
};

IK_PRIVATE_API IK_WARN_UNUSED ikret_t
ik_ntf_create(struct ik_ntf_t** ntf);

IK_PRIVATE_API void
ik_ntf_construct(struct ik_ntf_t* ntf);

IK_PRIVATE_API IK_WARN_UNUSED ikret_t
ik_ntf_from_nodes(struct ik_ntf_t* ntf, const struct ik_node_t* root);

C_END

#endif /* IK_NODE_TREE_FLATTENED */
