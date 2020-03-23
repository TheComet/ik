#ifndef IK_CONSTRAINT_H
#define IK_CONSTRAINT_H

#include "ik/config.h"
#include "ik/attachment.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_BEGIN

struct ik_constraint;
struct ik_node_data_t;

typedef void (*ik_constraint_apply_func)(struct ik_constraint* constraint,
                                         ikreal delta_rotation[4],
                                         const ikreal current_rotation[4]);

#define IK_CONSTRAINT_LIST \
    X(STIFF) \
    X(HINGE) \
    X(CONE) \
    X(CUSTOM)

enum ik_constraint_type
{
#define X(type) IK_CONSTRAINT_##type,
    IK_CONSTRAINT_LIST
#undef X

    IK_CONSTRAINT_TYPES_COUNT
};

struct ik_constraint
{
    IK_ATTACHMENT_HEAD

    /*!
     *
     */
    ik_constraint_apply_func apply;
    enum ik_constraint_type type;

    union {
        struct {
            union ik_quat angle;
        } stiff;
        struct {
            union ik_vec3 axis;
            ikreal min_angle;
            ikreal max_angle;
        } hinge;
        struct {
            union ik_vec3 center;
            ikreal max_angle;
        } cone;
        ikreal custom[5];
    } data;
};

#if defined(IK_BUILDING)

/*!
 * @brief Creates a new constraint object. It can be attached to any node in the
 * tree using ik_node_attach_constraint().
 */
IK_PUBLIC_API struct ik_constraint*
ik_constraint_create(void);

/*!
 * @brief Sets the type of constraint to enforce.
 * @note The tree must be rebuilt only if you change to or from the "stiff"
 * constraint (IK_CONSTRAINT_STIFF). Switching to any other constraint does not
 * require a rebuild. The reason for this is because the stiff constraint
 * causes the node to be excluded entirely from the chain tree, and determining
 * this requires a rebuild.
 */
IK_PUBLIC_API ikret
ik_constraint_set_type(struct ik_constraint* constraint, enum ik_constraint_type type);

/*!
 * @brief Allows the user to specify a custom callback function for enforcing
 * a constraint.
 */
IK_PUBLIC_API void
ik_constraint_set_custom(struct ik_constraint* constraint, ik_constraint_apply_func callback);

#endif /* IK_BUILDING */

C_END

#endif /* IK_CONSTRAINT_H */
