#include "ik/python/ik_type_ModuleRef.h"

struct ik_solver;

typedef struct ik_Solver
{
    ik_ModuleRef super;
    struct ik_solver* solver;
} ik_Solver;

extern PyTypeObject ik_SolverType;

int
init_ik_SolverType(void);
