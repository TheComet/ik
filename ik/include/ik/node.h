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

/*!
 * @brief Base structure used to build the tree to be solved.
 */
struct ik_node
{
    IK_REFCOUNTED_HEAD

    void* user_data;

    struct ik_algorithm* algorithm;
    struct ik_constraint* constraint;
    struct ik_effector* effector;
    struct ik_pole* pole;

    union ik_quat rotation;
    union ik_vec3 position;

    ikreal rotation_weight;
    ikreal mass;

    struct ik_node* parent;
    struct cs_vector children;  /* holds ik_node* objects */
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

/*!
 * @brief Attaches a node as a child to another node. The parent node gains
 * ownership of the child node and is responsible for deallocating it.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
IK_PUBLIC_API int
ik_node_link(struct ik_node* parent, struct ik_node* child);

IK_PUBLIC_API int
ik_node_can_link(const struct ik_node* parent, const struct ik_node* child);

/*!
 * @brief Unlinks a node from the tree, without freeing anything. All
 * children of the unlinked node remain in tact and will no longer be
 * affiliated with the original tree.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
IK_PUBLIC_API void
ik_node_unlink(struct ik_node* node);

IK_PUBLIC_API void
ik_node_unlink_all_children(struct ik_node* node);

IK_PUBLIC_API struct ik_node*
ik_node_find(struct ik_node* node, const void* user_data);

#define  ik_node_child_count(node) \
    vector_count(&node->children)

#define ik_node_get_child(node, idx) \
    (*(struct ik_node**)vector_get_element(&node->children, idx))

IK_PUBLIC_API uint32_t
ik_node_count(const struct ik_node* node);

/*!
 * @brief Reallocates all nodes and attachments into a flat array.
 */
IK_PUBLIC_API struct ik_node*
ik_node_pack(const struct ik_node* root);

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

#define NODE_FOR_EACH(node, child) \
    VECTOR_FOR_EACH(&(node)->children, struct ik_node*, p##child) \
    struct ik_node* child = *p##child; (void)child; {

#define NODE_END_EACH } VECTOR_END_EACH

C_END
