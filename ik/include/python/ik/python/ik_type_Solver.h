#pragma once

#include "ik/python/ik_type_ModuleRef.h"

struct ik_solver;
struct ik_Bone;

typedef struct ik_Solver
{
    ik_ModuleRef super;
    struct ik_Bone* root;  /* unfortunately have to store root node for repr() to work */
    struct ik_solver* solver;
} ik_Solver;

extern PyTypeObject ik_SolverType;

int
init_ik_SolverType(void);
