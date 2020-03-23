#include <gmock/gmock.h>
#include "ik/node.h"
#include "ik/effector.h"
#include "ik/pole.h"
#include "ik/constraint.h"
#include "ik/solver.h"

#define NAME solver

using namespace testing;

TEST(NAME, test)
{
    ik_node_t *tree, *n1, *n2, *n3;
    ik_node_create(&tree, 0);
    ik_node_create_child(&n1, tree, 1);
    ik_node_create_child(&n2, n1, 2);
    ik_node_create_child(&n3, n2, 3);
    n2->node_data->position = ik_vec3(0, 1, 0);
    n3->node_data->position = ik_vec3(0, 1, 0);

    ik_effector_t *eff1, *eff2;
    ik_effector_create(&eff1);
    ik_effector_create(&eff1);
    ik_effector_set_chain_length(eff1, 2);
    ik_effector_set_chain_length(eff2, 1);
    ik_node_attach(n2, eff1);
    ik_node_attach(n3, eff2);

    ik_pole* pole;
    ik_pole_create(&pole, IK_BLENDER);
    ik_pole_set_angle(pole, 45);
    ik_node_attach(n2, pole);

    ik_constraint *c1, *c2;
    ik_constraint_create(&c1, IK_HINGE);
    ik_constraint_create(&c2, IK_CONE);
    ik_constraint_set_rotation_limits(c1, 45, 90, 0, 0, 0, 0);
    ik_constraint_set_rotation_limits(c2, -20, 20, -30, 30, 0, 0);
    ik_node_attach(tree, c1);
    ik_node_attach(n1, c2);

    ik_solver_t *s1, *s2;
    ik_solver_create(&s1, IK_FABRIK);
    ik_solver_create(&s2, IK_ONE_BONE);
    ik_solver_set_max_iterations(s1, 10);
    ik_solver_enable_features(s1, IK_TARGET_ROTATIONS);
    ik_node_attach(tree, s1);
    ik_node_attach(n2, s2);

    ik_tasks_t* task_list;
    ik_tasks_create(&task_list);
    ik_tasks_pack_nodes(task_list, tree);
    ik_tasks_iterate_nodes(task_list, apply_scene_to_nodes_callback);
    ik_tasks_update_targets(task_list);
    ik_tasks_update_distances(task_list);
    ik_tasks_exec(task_list);
    ik_tasks_iterate_nodes(task_list, apply_nodes_to_scene_callback);

    while (game_is_running())
    {
        game_update_logic();
        game_update_animation();

        /* Update targets every frame. I'm using constant placeholder values here,
         * in a game these would be actively moving */
        ik_effector.set_target_position(eff1, ik_vec3(1, 2, -0.4));
        ik_effector.set_target_rotation(eff1, ik.quat_angle_vector(20, 1, 1, 0));
        ik_effector.set_target_position(eff2, ik_vec3(5, 7, 3));
        ik_pole.set_position(pole, ik_vec3(-5, 1, -5));

        ik_solver_iterate_nodes(solver, apply_scene_to_nodes_callback);
        ik_solver_update(solver);
        ik_solver_solve(solver);
        ik_solver_iterate_nodes(solver, apply_nodes_to_scene_callback);

        game_draw();
    }

    ik_solver_destroy(solver);
    ik_solver_destroy(a1);
    ik_solver_destroy(a2);
    ik_constraint_destroy(c1);
    ik_constraint_destroy(c2);
    ik_pole_destroy(pole);
    ik_effector_destroy(eff1);
    ik_effector_destroy(eff2);
    ik_node_destroy_recursive(tree);
}
