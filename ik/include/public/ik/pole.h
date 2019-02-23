#ifndef IK_POLE_H
#define IK_POLE_H

#include "ik/config.h"
#include "ik/refcount.h"
#include "ik/vec3.h"

#define IK_POLE_TYPE_LIST \
    X(GENERIC) \
    X(BLENDER) /* https://i.stack.imgur.com/lKN6o.jpg */ \
    X(MAYA)

C_BEGIN

struct ik_node_t;

enum ik_pole_type_e
{
#define X(arg) IK_POLE_##arg,
    IK_POLE_TYPE_LIST
#undef X

    IK_POLE_TYPE_COUNT
};

/*!
 * @brief Poles constrain the problem by an additional degree of freedom.
 */
struct ik_pole_t
{
    IK_REFCOUNTED(struct ik_pole_t)

    /* private stuff */
    void (*calculate_roll)(ikreal_t q[4], const struct ik_pole_t* pole);
    struct ik_node_data_t* node;
    struct ik_node_data_t* tip;

    /* public stuff */
    ikreal_t angle;
    union ik_vec3_t position;
};

#if defined(IK_BUILDING)

IK_PRIVATE_API struct ik_pole_t*
ik_pole_create(void);

IK_PRIVATE_API void
ik_pole_destroy(struct ik_pole_t* pole);

IK_PRIVATE_API void
ik_pole_set_type(struct ik_pole_t* pole, enum ik_pole_type_e type);

#endif /* IK_BUILDING */

C_END

#endif /* IK_POLE_H */
