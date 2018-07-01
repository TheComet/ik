#include "Python.h"
#include "ik/quat.h"

typedef struct ik_Quat
{
    PyObject_HEAD
    quat_t quat;
} ik_Quat;

extern PyTypeObject ik_QuatType;

int
init_ik_QuatType(void);
