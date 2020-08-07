#pragma once

#include "Python.h"

typedef struct ik_Log
{
    PyObject_HEAD

    PyObject* on_message;
    int severity;
    int timestamps;
} ik_Log;

extern PyTypeObject ik_LogType;

int
init_ik_LogType(void);
