#include "Python.h"

struct ik_algorithm_t;
typedef struct ik_Node ik_Node;

typedef struct ik_Solver
{
    PyObject_HEAD
    struct ik_algorithm_t* algorithm;
    ik_Node* tree;
} ik_Solver;

extern PyTypeObject ik_SolverType;

int
init_ik_SolverType(void);
