#include "ik/node_base.h"

enum node_FABRIK_constraint_marker_e
{
    CM_DEEPER    = 0x01,
    CM_SHALLOWER = 0x02
};

#define IK_NODE_FABRIK_HEAD                                                   \
    IK_NODE_HEAD                                                              \
                                                                              \
    /* Need to store the transform of each node before solving */             \
    union                                                                     \
    {                                                                         \
        struct                                                                \
        {                                                                     \
            ik_quat_t initial_rotation;                                       \
            ik_vec3_t initial_position;                                       \
        };                                                                    \
        ikreal_t initial_transform[7];                                        \
    };                                                                        \
                                                                              \
                                                                              \
    uint8_t constraint_markers;

struct ik_node_FABRIK_t
{
    IK_NODE_FABRIK_HEAD
};

IK_IMPLEMENT(node_FABRIK, node_base)
{
    IK_OVERRIDE(create)
    IK_CONSTRUCTOR(construct)
}
