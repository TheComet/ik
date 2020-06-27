#include "Python.h"
#include "ik/vec3.h"

#define ik_Vec3_CheckExact(op) \
    (Py_TYPE(op) == &ik_Vec3Type)

typedef struct ik_Vec3
{
    PyObject_HEAD
    union ik_vec3 vec;
} ik_Vec3;

extern PyTypeObject ik_Vec3Type;

int
init_ik_Vec3Type(void);

ik_Vec3*
vec3_ik_to_python(ikreal v[3]);
