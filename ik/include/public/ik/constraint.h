#ifndef IK_CONSTRAINT_H
#define IK_CONSTRAINT_H

#include "ik/config.h"
#include "ik/attachment.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_BEGIN

struct ik_constraint_t;
struct ik_node_data_t;

typedef void (*ik_constraint_apply_func)(ikreal_t delta_rotation[4],
                                         const ikreal_t current_rotation[4],
                                         struct ik_constraint_t* constraint);

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

    IK_CONSTRAINT_TYPES_COUNT
};

struct ik_constraint_t
{
    IK_ATTACHMENT_HEAD

    /*!
     *
     */
    ik_constraint_apply_func apply;
    enum ik_constraint_type_e type;

    union {
        struct {
            union ik_quat_t angle;
        } stiff;
        struct {
            union ik_vec3_t axis;
            ikreal_t min_angle;
            ikreal_t max_angle;
        } hinge;
        struct {
            union ik_vec3_t center;
            ikreal_t max_angle;
        } cone;
        ikreal_t custom[5];
    } data;
};

#if defined(IK_BUILDING)

/*!
 * @brief Creates a new constraint object. It can be attached to any node in the
 * tree using ik_node_attach_constraint().
 */
IK_PRIVATE_API ikret_t
ik_constraint_create(struct ik_constraint_t** constraint);

/*!
 * @brief Destroys and frees a constraint object. This should **NOT** be called
 * on constraints that are attached to nodes. Use ik_node_free_constraint()
 * instead.
 */
IK_PRIVATE_API void
ik_constraint_free(struct ik_constraint_t* constraint);

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

#endif /* IK_BUILDING */

C_END

#endif /* IK_CONSTRAINT_H */
