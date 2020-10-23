#pragma once

#include "ik/python/ik_type_TreeObject.h"

struct ik_Quat;
struct ik_Vec3;

typedef struct ik_Bone
{
    ik_TreeObject super;

    struct ik_Vec3* position;
    struct ik_Quat* rotation;
} ik_Bone;

extern PyTypeObject ik_BoneType;

int
init_ik_BoneType(void);
