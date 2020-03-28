#ifndef IK_SOLVER_GROUP_H
#define IK_SOLVER_GROUP_H

#include "ik/config.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_solver;

IK_PRIVATE_API struct ik_solver*
ik_solver_group_create(struct vector_t solver_list);

C_END

#endif /* IK_SOLVER_GROUP_H */
