#pragma once

#include "Python.h"

typedef struct ik_ModuleRef
{
    PyObject_HEAD
    PyObject* module;
} ik_ModuleRef;

extern PyTypeObject ik_ModuleRefType;

int
init_ik_ModuleRefType(void);
