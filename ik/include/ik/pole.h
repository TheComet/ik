#pragma once

#include "ik/config.h"
#include "ik/attachment.h"
#include "ik/vec3.h"

#define IK_POLE_TYPE_LIST \
    X(GENERIC)
#if 0
    X(BLENDER)  \
    X(MAYA)
#endif

C_BEGIN

struct ik_node;

/*!
 * @brief Poles constrain the problem by an additional degree of freedom.
 */
struct ik_pole
{
    IK_ATTACHMENT_HEAD

    /* private stuff */
    void (*calculate_roll)(const struct ik_pole* pole, ikreal q[4]);
    struct ik_node* base;
    struct ik_node* tip;

    /* public stuff */
    ikreal angle;
    union ik_vec3 position;
};

IK_PUBLIC_API struct ik_pole*
ik_pole_create(void);

IK_PUBLIC_API void
ik_pole_set_generic(struct ik_pole* pole);

IK_PUBLIC_API void
ik_pole_set_blender(struct ik_pole* pole);

IK_PUBLIC_API void
ik_pole_set_maya(struct ik_pole* pole);

IK_PUBLIC_API void
ik_pole_set_custom(struct ik_pole* pole, void(*calculate_roll)(const struct ik_pole*, ikreal q[4]));

IK_PUBLIC_API struct ik_pole*
ik_pole_duplicate(const struct ik_pole* pole);

IK_PUBLIC_API int
ik_pole_duplicate_from_tree(struct ik_tree_object* dst, const struct ik_tree_object* src);

C_END
