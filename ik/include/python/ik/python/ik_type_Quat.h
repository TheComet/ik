#include "Python.h"
#include "ik/quat.h"

#define ik_Quat_CheckExact(op) \
    (Py_TYPE(op) == &ik_QuatType)

typedef struct ik_Quat
{
    PyObject_HEAD
    union ik_quat quat;
} ik_Quat;

extern PyTypeObject ik_QuatType;

int
init_ik_QuatType(void);

int
quat_python_to_ik(PyObject* vpy, ikreal vik[4]);

ik_Quat*
quat_ik_to_python(ikreal q[4]);
