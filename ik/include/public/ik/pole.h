#ifndef IK_POLE_H
#define IK_POLE_H

#include "ik/config.h"

C_BEGIN

struct ik_node_t;

enum ik_pole_type_t
{
    IK_NONE = 0,
    IK_BLENDER,  /* https://i.stack.imgur.com/lKN6o.jpg */
    IK_MAYA
};

struct ik_pole_t
{
    /* public stuff */
    ikreal_t angle;
    uint16_t chain_length;

    /* "private" stuff */
    ikreal_t (*calculate_roll)(struct ik_pole_t* pole);
    struct ik_node_t* base_node;
    struct ik_node_t* tip_node;
};

IK_INTERFACE(pole_interface)
{
    struct ik_pole_t* 
    (*create)(enum ik_pole_type);

    void
    (*destroy)(struct ik_pole_t* pole);

    ikreal_t
    (*calculate_roll)(struct ik_pole_t* pole);
};

C_END

#endif /* IK_POLE_H */
