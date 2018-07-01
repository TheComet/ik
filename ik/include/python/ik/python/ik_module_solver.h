#include "Python.h"

struct ik_solver_t;
typedef struct ik_Node ik_Node;

typedef struct ik_Solver
{
    PyObject_HEAD
    struct ik_solver_t* solver;
    ik_Node* tree;
} ik_Solver;

extern PyTypeObject ik_SolverType;

int
init_ik_SolverType(void);
