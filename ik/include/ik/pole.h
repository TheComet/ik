#ifndef IK_POLE_H
#define IK_POLE_H

#include "ik/config.h"
#include "ik/attachment.h"
#include "ik/vec3.h"

#define IK_POLE_TYPE_LIST \
    X(GENERIC)
#if 0
    X(BLENDER) /* https://i.stack.imgur.com/lKN6o.jpg */ \
    X(MAYA)
#endif

C_BEGIN

struct ik_node;

enum ik_pole_type
{
#define X(arg) IK_POLE_##arg,
    IK_POLE_TYPE_LIST
#undef X

    IK_POLE_TYPE_COUNT
};

/*!
 * @brief Poles constrain the problem by an additional degree of freedom.
 */
struct ik_pole
{
    IK_ATTACHMENT_HEAD

    /* private stuff */
    void (*calculate_roll)(ikreal q[4], const struct ik_pole* pole);
    struct ik_node* base;
    struct ik_node* tip;

    /* public stuff */
    ikreal angle;
    union ik_vec3 position;
};

#if defined(IK_BUILDING)

IK_PRIVATE_API struct ik_pole*
ik_pole_create(void);

IK_PRIVATE_API void
ik_pole_set_type(struct ik_pole* pole, enum ik_pole_type type);

#endif /* IK_BUILDING */

C_END

#endif /* IK_POLE_H */
