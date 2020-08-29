#pragma once

#include "ik/python/ik_type_Vec3.h"
#include "ik/mat3x4.h"

#define ik_Mat3x4_CheckExact(op) \
    (Py_TYPE(op) == &ik_Mat3x4Type)

typedef struct ik_Mat3x4
{
    PyObject_HEAD
    union ik_mat3x4 mat;

    ik_Vec3* ex;
    ik_Vec3* ey;
    ik_Vec3* ez;
    ik_Vec3* et;
} ik_Mat3x4;

extern PyTypeObject ik_Mat3x4Type;

int
init_ik_Mat3x4Type(void);
