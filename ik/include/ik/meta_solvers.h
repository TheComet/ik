#pragma once

#include "ik/config.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_solver;
struct ik_node;

IK_PRIVATE_API struct ik_solver*
ik_solver_group_create(const struct cs_vector* solver_list);

IK_PRIVATE_API struct ik_solver*
ik_solver_combine_create(const struct cs_vector* solver_list, struct ik_node* shared_node);

C_END
