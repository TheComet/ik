#pragma once

#include "ik/tree_object.h"
#include "ik/tree_object_conversions.h"
#include "ik/quat.h"
#include "ik/vec3.h"

C_BEGIN

/*!
 * @brief Base structure used to build the tree to be solved.
 */
struct ik_node
{
    IK_TREE_OBJECT_HEAD

    union ik_quat rotation;
    union ik_vec3 position;
};

/*!
 * @brief Creates a new node and returns it. Each node requires a unique
 * identifier.
 * @param[out] node If successful, the address of new node is written to this
 * parameter.
 */
IK_PUBLIC_API struct ik_node*
ik_node_create(void);

/*!
 * @brief Creates a new node, attaches it as a child to the specified node,
 * and returns it. Each node requires a unique identifier, which can be used
 * later to search for nodes in the tree or to store user data for later access.
 */
IK_PUBLIC_API struct ik_node*
ik_node_create_child(struct ik_node* parent);

IK_PRIVATE_API void
ik_node_init(struct ik_node* node);

/*!
 * @brief Attaches a node as a child to another node. The parent node gains
 * ownership of the child node and is responsible for deallocating it.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
static inline int
ik_node_link(struct ik_node* parent, struct ik_node* child) {
    return ik_tree_object_link((struct ik_tree_object*)parent, (struct ik_tree_object*)child);
}

static inline int
ik_node_can_link(const struct ik_node* parent, const struct ik_node* child) {
    return ik_tree_object_can_link((const struct ik_tree_object*)parent, (struct ik_tree_object*)child);
}

/*!
 * @brief Unlinks a node from the tree, without freeing anything. All
 * children of the unlinked node remain in tact and will no longer be
 * affiliated with the original tree.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
static inline void
ik_node_unlink(struct ik_node* node) {
    ik_tree_object_unlink((struct ik_tree_object*)node);
}

static inline void
ik_node_unlink_all_children(struct ik_node* node) {
    ik_tree_object_unlink_all_children((struct ik_tree_object*)node);
}

static inline struct ik_node*
ik_node_find(struct ik_node* root, const void* user_data) {
    return (struct ik_node*)ik_tree_object_find((struct ik_tree_object*)root, user_data);
}

#define ik_node_child_count(node) \
    (ik_tree_object_child_count(node))

#define ik_node_get_child(node, idx) \
    ((struct ik_node*)ik_tree_object_get_child(node, idx))

static inline int
ik_node_count(const struct ik_node* root)  {
    return ik_tree_object_count((struct ik_tree_object*)root);
}

static inline int
ik_node_leaf_count(const struct ik_node* root)  {
    return ik_tree_object_leaf_count((struct ik_tree_object*)root);
}

/*!
 * @brief Reallocates all nodes into a flat array, but the new nodes will
 * still reference the same attachments.
 */
static inline struct ik_node*
ik_node_duplicate_shallow(const struct ik_node* root) {
    return (struct ik_node*)ik_tree_object_duplicate_shallow(
        (const struct ik_tree_object*)root, sizeof(*root), 0);
}

/*!
 * @brief Reallocates all nodes and attachments into a flat array.
 */
static inline struct ik_node*
ik_node_duplicate_full(const struct ik_node* root) {
    return (struct ik_node*)ik_tree_object_duplicate_full(
        (const struct ik_tree_object*)root, sizeof(*root), 0);
}

/*!
 * @brief The constraint is attached to the specified node.
 *
 * @note Constraints are a bit strange in how they are stored. They don't apply
 * to single nodes, rather, they apply to entire segments (edges connecting
 * nodes). This is not apparent in a single chain of nodes, but becomes apparent
 * if you consider a tree structure.
 *
 *    A   C
 *     \ /
 *      B
 *      |
 *      D
 *
 * If you wanted to constrain the rotation of D, then you would add a
 * constraint to node B. If you wanted to constraint the rotation of the
 * segment B-A then you would add a constraint to node A.
 *
 * @param[in] constraint The constraint object. The node gains ownership of
 * the constraint and is responsible for its deallocation. If the node already
 * owns a constraint, then the new constraint is ignored and an error code is
 * returned.
 * @param[in] node The child of the node you wish to constrain.
 * @return Returns IK_ALREADY_HAS_ATTACHMENT if the target node already has
 * a constraint attached. IK_OK if otherwise.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
#define X1(upper, lower, arg0)                                                \
        static inline struct ik_##lower*                                      \
        ik_node_create_##lower(struct ik_node* node, arg0 arg) {              \
            return ik_tree_object_create_##lower((struct ik_tree_object*)node, arg); \
        }
#define X(upper, lower)                                                       \
        static inline struct ik_##lower*                                      \
        ik_node_create_##lower(struct ik_node* node) {                        \
            return ik_tree_object_create_##lower((struct ik_tree_object*)node); \
        }
    IK_ATTACHMENT_LIST
#undef X
#undef X1

#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower)                                                       \
        static inline void                                                    \
        ik_node_attach_##lower(struct ik_node* node, struct ik_##lower* lower) { \
            ik_tree_object_attach_##lower((struct ik_tree_object*)node, lower); \
        }                                                                     \
                                                                              \
        static inline struct ik_##lower*                                      \
        ik_node_detach_##lower(struct ik_node* node) {                        \
            return ik_tree_object_detach_##lower((struct ik_tree_object*)node); \
        }
    IK_ATTACHMENT_LIST
#undef X
#undef X1

#define NODE_FOR_EACH_CHILD(node, child) \
    VECTOR_FOR_EACH(&(node)->children, struct ik_node*, p##child) \
    struct ik_node* child = *p##child; (void)child; {

#define NODE_END_EACH } VECTOR_END_EACH

C_END
