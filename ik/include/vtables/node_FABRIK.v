#include "ik/node_base.h"

#define IK_NODE_FABRIK_HEAD                                                   \
    IK_NODE_HEAD                                                              \
    union                                                                     \
    {                                                                         \
        struct                                                                \
        {                                                                     \
            ik_quat_t initial_rotation;                                       \
            ik_vec3_t initial_position;                                       \
        };                                                                    \
        ikreal_t initial_transform[7];                                        \
    };

struct ik_node_FABRIK_t
{
    IK_NODE_FABRIK_HEAD
};

IK_IMPLEMENT(node_FABRIK, node_base)
{
    IK_OVERRIDE(create)
    IK_CONSTRUCTOR(construct)
}
