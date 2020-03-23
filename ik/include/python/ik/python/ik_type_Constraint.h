#include "Python.h"

struct ik_constraint;

typedef struct ik_Constraint
{
    PyObject_HEAD
    struct ik_constraint* constraint;
} ik_Constraint;

extern PyTypeObject ik_ConstraintType;

int
init_ik_ConstraintType(void);
