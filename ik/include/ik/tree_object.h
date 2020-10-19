#pragma once

#include "ik/algorithm.h"
#include "ik/constraint.h"
#include "ik/effector.h"
#include "ik/pole.h"
#include "ik/config.h"
#include "ik/refcount.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_algorithm;
struct ik_constraint;
struct ik_effector;
struct ik_pole;
struct ik_tree_object;

#define IK_TREE_OBJECT_HEAD                                                   \
    IK_REFCOUNTED_HEAD                                                        \
                                                                              \
    void* user_data;                                                          \
                                                                              \
    struct ik_algorithm* algorithm;                                           \
    struct ik_constraint* constraint;                                         \
    struct ik_effector* effector;                                             \
    struct ik_pole* pole;                                                     \
                                                                              \
    struct ik_tree_object* parent;                                            \
    struct cs_vector children;  /* holds ik_tree_object* objects */           \
                                                                              \
    ikreal rotation_weight;                                                   \
    ikreal mass;

struct ik_tree_object
{
    IK_TREE_OBJECT_HEAD
};

/*!
 * @brief Creates a new node and returns it. Each node requires a unique
 * identifier.
 * @param[out] node If successful, the address of new node is written to this
 * parameter.
 */
IK_PRIVATE_API struct ik_tree_object*
ik_tree_object_create(uintptr_t derived_size);

/*!
 * @brief Creates a new node, attaches it as a child to the specified node,
 * and returns it. Each node requires a unique identifier, which can be used
 * later to search for nodes in the tree or to store user data for later access.
 */
IK_PRIVATE_API struct ik_tree_object*
ik_tree_object_create_child(struct ik_tree_object* parent, uintptr_t derived_size);

/*!
 * @brief Attaches a node as a child to another node. The parent node gains
 * ownership of the child node and is responsible for deallocating it.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
IK_PRIVATE_API int
ik_tree_object_link(struct ik_tree_object* parent, struct ik_tree_object* child);

IK_PRIVATE_API int
ik_tree_object_can_link(const struct ik_tree_object* parent, const struct ik_tree_object* child);

/*!
 * @brief Unlinks a node from the tree, without freeing anything. All
 * children of the unlinked node remain in tact and will no longer be
 * affiliated with the original tree.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
IK_PRIVATE_API void
ik_tree_object_unlink(struct ik_tree_object* node);

IK_PRIVATE_API void
ik_tree_object_unlink_all_children(struct ik_tree_object* node);

IK_PRIVATE_API struct ik_tree_object*
ik_tree_object_find(struct ik_tree_object* node, const void* user_data);

#define ik_tree_object_child_count(node) \
    (vector_count(&node->children))

#define ik_tree_object_get_child(node, idx) \
    (*(struct ik_tree_object**)vector_get_element(&node->children, idx))

/*!
 * @brief Sums up the total number of objects in the tree.
 */
IK_PRIVATE_API int
ik_tree_object_count(const struct ik_tree_object* node);

/*!
 * @brief Sums up the total number of leaf objects in the tree.
 */
IK_PRIVATE_API int
ik_tree_object_leaf_count(const struct ik_tree_object* node);

/*!
 * @brief Reallocates all nodes and attachments into a flat array.
 */
IK_PRIVATE_API struct ik_tree_object*
ik_tree_object_pack(const struct ik_tree_object* root, uintptr_t obj_size);

/*!
 * @brief Duplicates the tree, but the new tree will still reference all of the
 * old attachments (attachments are properly incref'd)
 */
IK_PRIVATE_API struct ik_tree_object*
ik_tree_object_duplicate_shallow(const struct ik_tree_object* root);

/*!
 * @brief Duplicates the tree and its attachments. The new tree will be fully
 * independent from the original tree.
 */
IK_PRIVATE_API struct ik_tree_object*
ik_tree_object_duplicate_full(const struct ik_tree_object* root);

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
        IK_PRIVATE_API struct ik_##lower*                                     \
        ik_tree_object_create_##lower(struct ik_tree_object* node, arg0 arg);
#define X(upper, lower)                                                       \
        IK_PRIVATE_API struct ik_##lower*                                     \
        ik_tree_object_create_##lower(struct ik_tree_object* node);
    IK_ATTACHMENT_LIST
#undef X
#undef X1

#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower)                                                       \
        IK_PRIVATE_API void                                                   \
        ik_tree_object_attach_##lower(struct ik_tree_object* node, struct ik_##lower* lower); \
                                                                              \
        IK_PRIVATE_API struct ik_##lower*                                     \
        ik_tree_object_detach_##lower(struct ik_tree_object* node);
    IK_ATTACHMENT_LIST
#undef X
#undef X1

#define TREE_OBJECT_FOR_EACH_CHILD(node, child) \
    VECTOR_FOR_EACH(&(node)->children, struct ik_tree_object*, p##child) \
    struct ik_tree_object* child = *p##child; (void)child; {

#define TREE_OBJECT_END_EACH } VECTOR_END_EACH

C_END
