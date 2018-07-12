#include "Python.h"

typedef struct ik_Pole
{
    PyObject_HEAD
} ik_Pole;

extern PyTypeObject ik_PoleType;

int
init_ik_PoleType(void);
