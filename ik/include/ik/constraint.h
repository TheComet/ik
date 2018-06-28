#ifndef IK_CONSTRAINT_H
#define IK_CONSTRAINT_H

#include "ik/config.h"

C_HEADER_BEGIN

struct ik_node_t;
struct ik_constraint_interface_t;

typedef int (*ik_constraint_apply_func)(struct ik_node_t*);

typedef enum ik_constraint_type_e
{
    IK_CONSTRAINT_NONE,
    IK_CONSTRAINT_STIFF,
    IK_CONSTRAINT_HINGE,
    IK_CONSTRAINT_CONE,
    IK_CONSTRAINT_CUSTOM
} ik_constraint_type_e;

struct ik_constraint_t
{
    const struct ik_constraint_interface_t* v;
    struct ik_node_t* node;
    ik_constraint_apply_func apply;
    ik_constraint_type_e type;
};

IK_INTERFACE(constraint_interface)
{
    /*!
     * @brief Creates a new constraint object. It can be attached to any node in the
     * tree using ik_node_attach_constraint().
     */
    struct ik_constraint_t*
    (*create)(ik_constraint_type_e constraint_type);

    /*!
     * @brief Sets the type of constraint to enforce.
     * @note The tree must be rebuilt only if you change to or from the "stiff"
     * constraint (IK_CONSTRAINT_STIFF). Switching to any other constraint does not
     * require a rebuild. The reason for this is because the stiff constraint
     * causes the node to be excluded entirely from the chain tree, and determining
     * this requires a rebuild.
     */
    void
    (*set)(struct ik_constraint_t* constraint, ik_constraint_type_e constraint_type);

    /*!
     * @brief Allows the user to specify a custom callback function for enforcing
     * a constraint.
     */
    void
    (*set_custom)(struct ik_constraint_t* constraint, ik_constraint_apply_func callback);

    /*!
     * @brief Destroys and frees a constraint object. This should **NOT** be called
     * on constraints that are attached to nodes. Use ik_node_destroy_constraint()
     * instead.
     */
    void
    (*destroy)(struct ik_constraint_t* constraint);

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
     * @note You will need to rebuild the solver's tree before solving.
     */
    ik_ret
    (*attach)(struct ik_constraint_t* constraint, struct ik_node_t* node);

    /*!
     * @brief Removes the constraint from the node it is attached to, if it exists.
     * The field node->constraint is set to NULL.
     * @note You regain ownership of the object and must destroy it manually when
     * done with it. You may also attach it to another node.
     * @note You will need to rebuild the solver's tree before solving.
     */
    void
    (*detach)(struct ik_constraint_t* constraint);
};

C_HEADER_END

#endif /* IK_CONSTRAINT_H */
