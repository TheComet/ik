#ifndef IK_CONSTRAINT_H
#define IK_CONSTRAINT_H

#include "ik/config.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_BEGIN

struct ik_node_t;

typedef void (*ik_constraint_apply_func)(const struct ik_node_t* node, ikreal_t compensate_rotation[4]);

#define IK_CONSTRAINTS_LIST \
    X(STIFF) \
    X(HINGE) \
    X(CONE) \
    X(CUSTOM)

enum ik_constraint_type_e
{
#define X(type) IK_CONSTRAINT_##type,
    IK_CONSTRAINTS_LIST
#undef X
};

struct ik_constraint_t
{
    struct ik_node_t* node;
    ik_constraint_apply_func apply;
    enum ik_constraint_type_e type;

    union {
        struct {
            struct ik_quat_t angle;
        } stiff;
        struct {
            struct ik_vec3_t axis;
            ikreal_t min_angle;
            ikreal_t max_angle;
        } hinge;
        struct {
            struct ik_vec3_t center;
            ikreal_t max_angle;
        } cone;
        ikreal_t custom[5];
    };
};

#if defined(IK_BUILDING)

/*!
 * @brief Creates a new constraint object. It can be attached to any node in the
 * tree using ik_node_attach_constraint().
 */
IK_PRIVATE_API struct ik_constraint_t*
ik_constraint_create(void);

/*!
 * @brief Destroys and frees a constraint object. This should **NOT** be called
 * on constraints that are attached to nodes. Use ik_node_destroy_constraint()
 * instead.
 */
IK_PRIVATE_API void
ik_constraint_destroy(struct ik_constraint_t* constraint);

/*!
 * @brief Duplicates the specified constraint and returns it.
 */
IK_PRIVATE_API struct ik_constraint_t*
ik_constraint_duplicate(const struct ik_constraint_t* constraint);

/*!
 * @brief Sets the type of constraint to enforce.
 * @note The tree must be rebuilt only if you change to or from the "stiff"
 * constraint (IK_CONSTRAINT_STIFF). Switching to any other constraint does not
 * require a rebuild. The reason for this is because the stiff constraint
 * causes the node to be excluded entirely from the chain tree, and determining
 * this requires a rebuild.
 */
IK_PRIVATE_API ikret_t
ik_constraint_set_type(struct ik_constraint_t* constraint, enum ik_constraint_type_e constraint_type);

/*!
 * @brief Allows the user to specify a custom callback function for enforcing
 * a constraint.
 */
IK_PRIVATE_API void
ik_constraint_set_custom(struct ik_constraint_t* constraint, ik_constraint_apply_func callback);

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
IK_PRIVATE_API ikret_t
ik_constraint_attach(struct ik_constraint_t* constraint, struct ik_node_t* node);

/*!
 * @brief Removes the constraint from the node it is attached to, if it exists.
 * The field node->constraint is set to NULL.
 * @note You regain ownership of the object and must destroy it manually when
 * done with it. You may also attach it to another node.
 * @note You will need to rebuild the solver's tree before solving.
 */
IK_PRIVATE_API void
ik_constraint_detach(struct ik_constraint_t* constraint);

#endif /* IK_BUILDING */

C_END

#endif /* IK_CONSTRAINT_H */
