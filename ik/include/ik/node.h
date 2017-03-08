#ifndef IK_NODE_H
#define IK_NODE_H

#include "ik/config.h"
#include "ik/pstdint.h"
#include "ik/bst_vector.h"
#include "ik/vector3.h"
#include "ik/quaternion.h"

C_HEADER_BEGIN

struct effector_t;

struct node_t
{
    void* user_data;
    uint32_t guid;
    struct vector3_t position;
    struct quaternion_t rotation;

    struct node_t* parent;
    struct bstv_t children;
    struct effector_t* effector;

};

IK_PUBLIC_API struct node_t*
node_create(uint32_t guid);

IK_PUBLIC_API void
node_construct(struct node_t* node, uint32_t guid);

IK_PUBLIC_API void
node_destruct(struct node_t* node);

IK_PUBLIC_API void
node_destroy(struct node_t* node);

IK_PUBLIC_API void
node_add_child(struct node_t* node, struct node_t* child);

IK_PUBLIC_API void
node_remove_child(struct node_t* node);

IK_PUBLIC_API struct node_t*
node_find_child(struct node_t* node, uint32_t guid);

IK_PUBLIC_API void
node_attach_effector(struct node_t* node, struct effector_t* effector);

IK_PUBLIC_API void
node_dump_to_dot(struct node_t* node, const char* file_name);

C_HEADER_END

#endif /* IK_NODE_H */
