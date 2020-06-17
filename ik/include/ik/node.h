#ifndef IK_NODE_H
#define IK_NODE_H

#include "ik/algorithm.h"
#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/pole.h"
#include "ik/config.h"
#include "ik/refcount.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include "cstructures/btree.h"

#define NODE_FOR_EACH(node, userdata, child) \
    BTREE_FOR_EACH(&(node)->children, struct ik_node*, userdata, p##child) \
    struct ik_node* child = *p##child; (void)child; {

#define NODE_END_EACH } BTREE_END_EACH

C_BEGIN

struct ik_algorithm;
struct ik_constraint;
struct ik_effector;
struct ik_pole;

union ik_node_user_data {
    void* ptr;
    uintptr_t guid;
};

/*!
 * @brief Base structure used to build the tree to be solved.
 */
struct ik_node
{
    IK_REFCOUNTED_HEAD

    union ik_node_user_data user;

    struct ik_algorithm* algorithm;
    struct ik_constraint* constraint;
    struct ik_effector* effector;
    struct ik_pole* pole;

    union ik_quat rotation;
    union ik_vec3 position;

    ikreal dist_to_parent;
    ikreal rotation_weight;
    ikreal mass;

    struct ik_node* parent;
    struct btree_t children;  /* holds ik_node* objects */
};

/*!
 * @brief Creates a new node and returns it. Each node requires a unique
 * identifier.
 * @param[out] node If successful, the address of new node is written to this
 * parameter.
 * @param[in] id Must be a unique value identifying this node. If you are using
 * this library with an existing scene graph, then you can specify the pointer
 * to the corresponding C++ node for this parameter. It can also be a random
 * integer value. As long as it is globally unique. You may use ik_guid() to
 * generate a random integer if you don't care.
 */
IK_PUBLIC_API struct ik_node*
ik_node_create(union ik_node_user_data user);

/*!
 * @brief Creates a new node, attaches it as a child to the specified node,
 * and returns it. Each node requires a unique identifier, which can be used
 * later to search for nodes in the tree or to store user data for later access.
 */
IK_PUBLIC_API struct ik_node*
ik_node_create_child(struct ik_node* parent, union ik_node_user_data user);

/*!
 * @brief Attaches a node as a child to another node. The parent node gains
 * ownership of the child node and is responsible for deallocating it.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
IK_PUBLIC_API int
ik_node_link(struct ik_node* parent, struct ik_node* child);

/*!
 * @brief Unlinks a node from the tree, without freeing anything. All
 * children of the unlinked node remain in tact and will no longer be
 * affiliated with the original tree.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
IK_PUBLIC_API void
ik_node_unlink(struct ik_node* node);

static inline uint32_t
ik_node_child_count(const struct ik_node* node) {
    return btree_count(&node->children);
}

IK_PUBLIC_API uint32_t
ik_node_count(const struct ik_node* node);

/*!
 * @brief Reallocates all nodes and attachments into a flat array.
 */
IK_PUBLIC_API struct ik_node*
ik_node_pack(const struct ik_node* root);

/*!
 * @brief Searches recursively for a node in a tree with the specified global
 * identifier.
 * @return Returns NULL if the node was not found, otherwise the node is
 * returned.
 */
IK_PUBLIC_API struct ik_node*
ik_node_find(struct ik_node* node, union ik_node_user_data user);

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
        IK_PUBLIC_API struct ik_##lower*                                      \
        ik_node_create_##lower(struct ik_node* node, arg0 arg);
#define X(upper, lower)                                                       \
        IK_PUBLIC_API struct ik_##lower*                                      \
        ik_node_create_##lower(struct ik_node* node);
    IK_ATTACHMENT_LIST
#undef X
#undef X1

#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower)                                                       \
        IK_PUBLIC_API void                                                    \
        ik_node_attach_##lower(struct ik_node* node, struct ik_##lower* lower); \
                                                                              \
        IK_PUBLIC_API struct ik_##lower*                                      \
        ik_node_detach_##lower(struct ik_node* node);
    IK_ATTACHMENT_LIST
#undef X
#undef X1

static inline union ik_node_user_data
ik_guid(uint32_t guid) {
    union ik_node_user_data user;
    user.guid = guid;
    return user;
}

static inline union ik_node_user_data
ik_ptr(void* ptr) {
    union ik_node_user_data user;
    user.ptr = ptr;
    return user;
}

C_END

#endif /* IK_NODE_H */
