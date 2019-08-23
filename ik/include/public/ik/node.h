#ifndef IK_NODE_H
#define IK_NODE_H

#include "cstructures/btree.h"
#include "ik/config.h"
#include "ik/node_data.h"

C_BEGIN

/*!
 * @brief Base structure used to build the tree to be solved.
 */
struct ik_node_t
{
    struct ik_node_data_t* d;
    struct ik_node_t* parent;
    struct btree_t children;  /* holds ik_node_t* objects */
    uint32_t data_index;
};

/*!
 * @brief Creates a new node and returns it. Each node requires a unique
 * identifier.
 * @param[out] node If successful, the new node is written to this parameter.
 * @param[in] cpp_object_or_uid The value set here is the value that will be
 * passed to the ik_node_read_callback() and ik_node_write_callback)() functions.
 * If you are using this library with an existing scene graph, then you can
 * specify the pointer to the corresponding C++ node for this parameter.
 * It can also be a random integer value. As long as it is globally unique.
 * You may use ik_guid() to generate a random integer if you don't care.
 */
IKRET
ik_node_create(struct ik_node_t** node, void* user_data);

/*!
 * @brief Constructs an already allocated node.
 */
IKRET
ik_node_init(struct ik_node_t* node, void* user_data);

/*!
 * @brief Destructs a node, freeing all children in the process, but does
 * not deallocate the node object itself.
 */
void
ik_node_deinit(struct ik_node_t* node);

/*!
 * @brief Destructs and frees the specified node. All children will be
 * transferred to the parent node.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
void
ik_node_free(struct ik_node_t* node);

/*!
 * @brief Destructs a node, freeing all children in the process, but does
 * not deallocate the node object itself.
 */
void
ik_node_deinit_recursive(struct ik_node_t* node);

/*!
 * @brief Destructs and frees the specified node. All children will be
 * transferred to the parent node.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
void
ik_node_free_recursive(struct ik_node_t* node);

/*!
 * @brief Creates a new node, attaches it as a child to the specified node,
 * and returns it. Each node requires a unique identifier, which can be used
 * later to search for nodes in the tree or to store user data for later access.
 */
IKRET
ik_node_create_child(struct ik_node_t** child,
                     struct ik_node_t* parent,
                     void* user_data);

/*!
 * @brief Attaches a node as a child to another node. The parent node gains
 * ownership of the child node and is responsible for deallocating it.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
IKRET
ik_node_link(struct ik_node_t* parent,
             struct ik_node_t* child);

/*!
 * @brief Unlinks a node from the tree, without freeing anything. All
 * children of the unlinked node remain in tact and will no longer be
 * affiliated with the original tree.
 * @note You will need to rebuild the algorithm's tree before solving.
 */
void
ik_node_unlink(struct ik_node_t* node);

/*!
 * @brief Searches recursively for a node in a tree with the specified global
 * identifier.
 * @return Returns NULL if the node was not found, otherwise the node is
 * returned.
 */
struct ik_node_t*
ik_node_find(struct ik_node_t* node, const void* user_data);

/*!
 * @brief Creates and attaches a new effector to the specified node.
 * @param[out] eff If the function succeeds, a pointer to the new effector
 * object is written to this parameter.
 * @note ik_effector_t is refcounted. The pointer returned is a borrowed
 * reference, i.e. the refcount is not incremented. If you wish to use the
 * effector elsewhere and are unsure if the node will outlive the effector, you
 * should IK_INCREF the effector.
 * @param[in] node The node to create the effector on.
 * @return Returns IK_OK if the effector is successfully created. If the node
 * already has an effector, then IK_ERR_ALREADY_HAS_ATTACHMENT is returned. If
 * there is not enough memory to allocate the effector, then IK_ERR_OUT_OF_MEMORY
 * is returned.
 */
#define X(upper, lower, type)                                                 \
        IKRET                                                                 \
        ik_node_create_##lower(type** a, struct ik_node_t* node);             \
                                                                              \
        IKRET                                                                 \
        ik_node_attach_##lower(struct ik_node_t* node, type* a);              \
                                                                              \
        void                                                                  \
        ik_node_release_##lower(struct ik_node_t* node);                      \
                                                                              \
        type*                                                                 \
        ik_node_take_##lower(struct ik_node_t* node);
    IK_ATTACHMENT_LIST
#undef X

#define IK_NODE_CONSTRAINT(n)       ((n)->d->constraint[(n)->data_index])
#define IK_NODE_ALGORITHM(n)        ((n)->d->algorithm[(n)->data_index])
#define IK_NODE_EFFECTOR(n)         ((n)->d->effector[(n)->data_index])
#define IK_NODE_POLE(n)             ((n)->d->pole[(n)->data_index])
#define IK_NODE_USER_DATA(n)        ((n)->d->user_data[(n)->data_index])
#define IK_NODE_TRANSFORM(n)        ((n)->d->transform[(n)->data_index])
#define IK_NODE_DIST_TO_PARENT(n)   ((n)->d->dist_to_parent[(n)->data_index])
#define IK_NODE_ROTATION_WEIGHT(n)  ((n)->d->rotation_weight[(n)->data_index])
#define IK_NODE_MASS(n)             ((n)->d->mass[(n)->data_index])
#define IK_NODE_CHILD_COUNT(n)      (btree_count(&(n)->children))

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

#define NODE_FOR_EACH(node, userdata, child) \
    BTREE_FOR_EACH(&(node)->children, struct ik_node_t*, userdata, p##child) \
    struct ik_node_t* child = *p##child; (void)child; {

#define NODE_END_EACH } BTREE_END_EACH

C_END

#endif /* IK_NODE_H */
