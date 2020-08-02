#pragma once

#include "ik/config.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_solver;

IK_PRIVATE_API struct ik_solver*
ik_solver_group_create(struct cs_vector solver_list);

IK_PRIVATE_API struct ik_solver*
ik_solver_combine_create(struct cs_vector solver_list);

C_END
