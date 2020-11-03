#pragma once

#include "ik/python/ik_type_TreeObject.h"

struct ik_Quat;
struct ik_Vec3;

typedef struct ik_Node
{
    ik_TreeObject super;

    struct ik_Vec3* position;
    struct ik_Quat* rotation;
} ik_Node;

extern PyTypeObject ik_NodeType;

int
init_ik_NodeType(void);
