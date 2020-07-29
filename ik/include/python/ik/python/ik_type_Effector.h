#pragma once

#include "ik/python/ik_type_Attachment.h"

#define ik_Effector_CheckExact(o) \
    (Py_TYPE(o) == &ik_EffectorType)

struct ik_Vec3;
struct ik_Quat;

typedef struct ik_Effector
{
    ik_Attachment super;
    struct ik_Vec3* target_position;
    struct ik_Quat* target_rotation;
} ik_Effector;

extern PyTypeObject ik_EffectorType;

int
init_ik_EffectorType(void);
