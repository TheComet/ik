#include "Python.h"

struct ik_constraint_t;

typedef struct ik_Constraint
{
    PyObject_HEAD
    struct ik_constraint_t* constraint;
} ik_Constraint;

extern PyTypeObject ik_ConstraintType;

int
init_ik_ConstraintType(void);
