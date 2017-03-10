#ifndef IK_NODE_H
#define IK_NODE_H

#include "ik/gen/config.h"
#include "ik/pstdint.h"
#include "ik/bst_vector.h"
#include "ik/vec3.h"
#include "ik/quat.h"

C_HEADER_BEGIN

struct ik_effector_t;

struct ik_node_t
{
    void* user_data;
    uint32_t guid;
    struct vec3_t position;
    struct quat_t rotation;

    struct vec3_t solved_position;
    struct quat_t solved_rotation;
    ik_real segment_length;

    struct ik_node_t* parent;
    struct bstv_t children;
    struct ik_effector_t* effector;
};

IK_PUBLIC_API struct ik_node_t*
ik_node_create(uint32_t guid);

IK_PUBLIC_API void
ik_node_construct(struct ik_node_t* node, uint32_t guid);

IK_PUBLIC_API void
ik_node_destruct(struct ik_node_t* node);

IK_PUBLIC_API void
ik_node_destroy(struct ik_node_t* node);

IK_PUBLIC_API void
ik_node_add_child(struct ik_node_t* node, struct ik_node_t* child);

IK_PUBLIC_API void
ik_node_remove_child(struct ik_node_t* node);

IK_PUBLIC_API struct ik_node_t*
ik_node_find_child(struct ik_node_t* node, uint32_t guid);

IK_PUBLIC_API void
ik_node_attach_effector(struct ik_node_t* node, struct ik_effector_t* effector);

IK_PUBLIC_API void
ik_node_dump_to_dot(struct ik_node_t* node, const char* file_name);

C_HEADER_END

#endif /* IK_NODE_H */
