#include "Python.h"
#include "ik/vec3.h"

typedef struct ik_Vec3
{
    PyObject_HEAD
    union ik_vec3 vec;
} ik_Vec3;

extern PyTypeObject ik_Vec3Type;

int
init_ik_Vec3Type(void);
