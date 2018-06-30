#include "Python.h"
#include "ik/solver.h"

typedef struct ik_Solver
{
    PyObject_HEAD
} ik_Solver;

extern PyTypeObject ik_SolverType;

int
init_ik_SolverType(void);
