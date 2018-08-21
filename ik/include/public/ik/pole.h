#ifndef IK_POLE_H
#define IK_POLE_H

#include "ik/config.h"
#include "ik/vec3.h"

C_BEGIN

struct ik_node_t;

enum ik_pole_type_e
{
    IK_GENERIC = 0,
    IK_BLENDER,  /* https://i.stack.imgur.com/lKN6o.jpg */
    IK_MAYA
};

struct ik_pole_t
{
    /* private stuff */
    void (*calculate_roll)(ikreal_t q[4], const struct ik_pole_t* pole);
    struct ik_node_t* node;
    struct ik_node_t* tip;

    /* public stuff */
    ikreal_t angle;
    struct ik_vec3_t position;
};

IK_PUBLIC_API struct ik_pole_t*
ik_pole_create(void);

IK_PUBLIC_API void
ik_pole_destroy(struct ik_pole_t* pole);

IK_PUBLIC_API void
ik_pole_set_type(struct ik_pole_t* pole, enum ik_pole_type_e type);

/*!
 * @brief Duplicates the specified pole object and returns it.
 */
IK_PUBLIC_API struct ik_pole_t*
ik_pole_duplicate(const struct ik_pole_t* pole);

/*!
 * @brief Attaches an pole object to the node. The node gains ownership
 * of the pole and is responsible for its deallocation. If the node
 * already owns a pole, then it is first destroyed.
 * @return Returns IK_ALREADY_HAS_ATTACHMENT if the target node already has
 * a pole attached. IK_OK if otherwise.
 * @note You will need to rebuild the solver's tree before solving.
 */
IK_PUBLIC_API ikret_t
ik_pole_attach(struct ik_pole_t* pole, struct ik_node_t* node);

/*!
 * @brief Removes the pole from the node it is attached to, if it exists.
 * The field node->pole is set to NULL.
 * @note You regain ownership of the object and must destroy it manually when
 * done with it. You may also attach it to another node.
 * @note You will need to rebuild the solver's tree before solving.
 */
IK_PUBLIC_API void
ik_pole_detach(struct ik_pole_t* pole);

C_END

#endif /* IK_POLE_H */
