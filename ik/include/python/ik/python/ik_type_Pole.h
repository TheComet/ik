#pragma once

#include "ik/python/ik_type_Attachment.h"

#define ik_Pole_CheckExact(o) \
    (Py_TYPE(o) == &ik_PoleType)

struct ik_Vec3;

typedef struct ik_Pole
{
    ik_Attachment super;

    struct ik_Vec3* position;
} ik_Pole;

extern PyTypeObject ik_PoleType;
extern PyTypeObject ik_GenericPoleType;
extern PyTypeObject ik_BlenderPoleType;
extern PyTypeObject ik_MayaPoleType;

int
init_ik_PoleType(void);
