#pragma once

#include "ik/python/ik_type_ModuleRef.h"

struct ik_pose;

typedef struct ik_Pose
{
    ik_ModuleRef super;
    struct ik_pose* pose;
} ik_Pose;

extern PyTypeObject ik_PoseType;

int
init_ik_PoseType(void);
