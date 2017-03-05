#ifndef IK_SOLVER_FABRIK_H
#define IK_SOLVER_FABRIK_H

#include "ik/config.h"
#include "ik/ordered_vector.h"
#include "ik/solver.h"

C_HEADER_BEGIN

struct node_t;

struct FABRIK_data_t
{
    SOLVER_DATA_HEAD
    struct ordered_vector_t chain_list;  /* chain_t structures */
};

struct fabrik_t
{
    union
    {
        struct solver_data_t solver;
        struct FABRIK_data_t fabrik;
    } base;
};

struct solver_t*
solver_FABRIK_create(void);

struct solver_t*
solver_FABRIK_destroy(struct solver_t* solver);

char
solver_FABRIK_solve(struct solver_t* solver);

C_HEADER_END

#endif /* IK_SOLVER_FABRIK_H */
