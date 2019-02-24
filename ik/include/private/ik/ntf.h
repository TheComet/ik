#ifndef IK_NODE_TREE_FLATTENED
#define IK_NODE_TREE_FLATTENED

#include "ik/config.h"

C_BEGIN

struct btree_t;
struct vector_t;
struct ik_node_data_t;
struct ik_node_t;

struct ik_ntf_index_data_t
{
    uint32_t pre_node;         /* Index to current pre-order node */
    uint32_t post_node;        /* Index to current post-order node */
    uint32_t pre_base;         /* Index to current pre-order node's direct base node */
    uint32_t post_base;        /* Index to current post-order node's direct base node */
    uint16_t pre_child_count;  /* Current pre-order node's child count */
    uint16_t post_child_count; /* Current post-order node's child count */
};

struct ik_ntf_t
{
    /*
     * Contiguous array of node_data_t objects. All node_data refcounts point
     * to the same refcount, because in a sense, all nodes own each other's
     * memory (i.e. all node_data instances must be unref'd before the
     * congiguous block of memory can be freed).
     */
    struct ik_node_data_t* node_data;

    struct ik_ntf_index_data_t* indices;

    uint32_t node_count;
};

IK_PRIVATE_API IK_WARN_UNUSED ikret_t
ik_ntf_create(struct ik_ntf_t** ntf,
              struct ik_node_t* subtree_root,
              const struct btree_t* marked_nodes);

IK_PRIVATE_API IK_WARN_UNUSED ikret_t
ik_ntf_construct(struct ik_ntf_t* ntf,
                 struct ik_node_t* subtree_root,
                 const struct btree_t* marked_nodes);

IK_PRIVATE_API void
ik_ntf_destruct(struct ik_ntf_t* ntf);

IK_PRIVATE_API void
ik_ntf_destroy(struct ik_ntf_t* ntf);

IK_PRIVATE_API uint32_t
ik_ntf_find_highest_child_count(const struct ik_ntf_t* ntf);

IK_PRIVATE_API IK_WARN_UNUSED ikret_t
ik_ntf_list_create(struct vector_t** ntf_list);

IK_PRIVATE_API void
ik_ntf_list_construct(struct vector_t* ntf_list);

IK_PRIVATE_API IK_WARN_UNUSED ikret_t
ik_ntf_list_fill(struct vector_t* ntf_list, struct ik_node_t* root);

IK_PRIVATE_API void
ik_ntf_list_clear(struct vector_t* ntf_list);

IK_PRIVATE_API void
ik_ntf_list_destroy(struct vector_t* ntf_list);

#define NTF_PRE_NODE(ntf, index) \
    (&(ntf)->node_data[(ntf)->indices[index].pre_node])

#define NTF_POST_NODE(ntf, index) \
    (&(ntf)->node_data[(ntf)->indices[index].post_node])

#define NTF_PRE_BASE(ntf, index) \
    (&(ntf)->node_data[(ntf)->indices[index].pre_base])

#define NTF_POST_BASE(ntf, index) \
    (&(ntf)->node_data[(ntf)->indices[index].post_node])

#define NTF_PRE_CHILD_COUNT(ntf, index) \
    ((ntf)->indices[i].pre_child_count)

#define NTF_POST_CHILD_COUNT(ntf, index) \
    ((ntf)->indices[i].post_child_count)

#define NTF_FOR_EACH(ntf_list, ntf) \
    VECTOR_FOR_EACH(ntf_list, struct ik_ntf_t, ntf)

#define NTF_END_EACH \
    VECTOR_END_EACH

C_END

#endif /* IK_NODE_TREE_FLATTENED */
