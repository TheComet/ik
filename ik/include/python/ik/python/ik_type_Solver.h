#include "Python.h"

struct ik_algorithm;
struct ik_Node;

typedef struct ik_Solver
{
    PyObject_HEAD
    struct ik_algorithm* algorithm;
    struct ik_Node* tree;
} ik_Solver;

extern PyTypeObject ik_SolverType;

int
init_ik_SolverType(void);
