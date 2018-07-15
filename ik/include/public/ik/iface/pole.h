#ifndef IK_POLE_H
#define IK_POLE_H

#include "ik/config.h"
#include "ik/iface/vec3.h"

C_BEGIN

struct ik_node_t;
struct ik_pole_interface_t;

enum ik_pole_type_e
{
    IK_GENERIC = 0,
    IK_BLENDER,  /* https://i.stack.imgur.com/lKN6o.jpg */
    IK_MAYA
};

struct ik_pole_t
{
    /* private stuff */
    const struct ik_pole_interface_t* v;
    void (*calculate_roll)(ikreal_t q[4], const struct ik_pole_t* pole);
    struct ik_node_t* node;
    struct ik_node_t* tip;

    /* public stuff */
    ikreal_t angle;
    struct ik_vec3_t position;
};

IK_INTERFACE(pole_interface)
{
    struct ik_pole_t*
    (*create)(void);

    void
    (*destroy)(struct ik_pole_t* pole);

    void
    (*set_type)(struct ik_pole_t* pole, enum ik_pole_type_e type);

    /*!
     * @brief Duplicates the specified pole object and returns it.
     */
    struct ik_pole_t*
    (*duplicate)(const struct ik_pole_t* pole);

    /*!
     * @brief Attaches an pole object to the node. The node gains ownership
     * of the pole and is responsible for its deallocation. If the node
     * already owns a pole, then it is first destroyed.
     * @return Returns IK_ALREADY_HAS_ATTACHMENT if the target node already has
     * a pole attached. IK_OK if otherwise.
     * @note You will need to rebuild the solver's tree before solving.
     */
    ikret_t
    (*attach)(struct ik_pole_t* pole, struct ik_node_t* node);

    /*!
     * @brief Removes the pole from the node it is attached to, if it exists.
     * The field node->pole is set to NULL.
     * @note You regain ownership of the object and must destroy it manually when
     * done with it. You may also attach it to another node.
     * @note You will need to rebuild the solver's tree before solving.
     */
    void
    (*detach)(struct ik_pole_t* pole);
};

C_END

#endif /* IK_POLE_H */
