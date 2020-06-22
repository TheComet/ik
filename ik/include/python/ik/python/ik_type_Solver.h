#include "Python.h"

struct ik_solver;

typedef struct ik_Solver
{
    PyObject_HEAD
    struct ik_solver* solver;
} ik_Solver;

extern PyTypeObject ik_SolverType;

int
init_ik_SolverType(void);
