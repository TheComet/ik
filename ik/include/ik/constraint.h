#ifndef IK_CONSTRAINT_H
#define IK_CONSTRAINT_H

#include "ik/config.h"

C_HEADER_BEGIN

typedef struct ik_node_t ik_node_t;
typedef struct ik_constraint_t ik_constraint_t;

typedef struct ik_constraint_t
{
    void* user_data;
} ik_constraint_t;

/*!
 * @brief Creates a new constraint object. It can be attached to any node in the
 * tree using ik_node_attach_constraint().
 */
IK_PUBLIC_API ik_constraint_t*
ik_constraint_create(void);

/*!
 * @brief Constructs a previously allocated constraint object.
 */
IK_PUBLIC_API void
ik_constraint_construct(ik_constraint_t* constraint);

/*!
 * @brief Destroys and frees a constraint object. This should **NOT** be called
 * on constraints that are attached to nodes. Use ik_node_destroy_constraint()
 * instead.
 */
IK_PUBLIC_API void
ik_constraint_destroy(ik_constraint_t* constraint);

C_HEADER_END

#endif /* IK_CONSTRAINT_H */
