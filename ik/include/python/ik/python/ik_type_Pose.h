#pragma once

#include "Python.h"

struct ik_pose;

typedef struct ik_Pose
{
    PyObject_HEAD

    struct ik_pose* pose;
} ik_Pose;

extern PyTypeObject ik_PoseType;

int
init_ik_PoseType(void);
