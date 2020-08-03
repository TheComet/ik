#include "ik/log.h"
#include "ik/solver.h"
#include "ik/meta_solvers.h"
#include "ik/node.h"
#include "ik/quat.inl"
#include "cstructures/memory.h"
#include <assert.h>
#include <string.h>

struct ik_solver_combine
{
    IK_SOLVER_HEAD

    struct ik_solver** subsolvers;
    struct ik_node** child_nodes;
    struct ik_node* shared_node;
    union ik_quat* child_rotations;

    int subsolver_count;
    int child_node_count;
};

/* ------------------------------------------------------------------------- */
static int
solver_combine_init(struct ik_solver* solver_base, const struct ik_subtree* subtree)
{
    ik_log_printf(IK_ERROR, "solver_combine_init() called! This should not happen ever!");
    assert(0);
    return -1;
}

/* ------------------------------------------------------------------------- */
static void
solver_combine_deinit(struct ik_solver* solver_base)
{
    int i;
    struct ik_solver_combine* solver = (struct ik_solver_combine*)solver_base;

    for (i = 0; i != solver->child_node_count; ++i)
        IK_DECREF(solver->child_nodes[i]);
    for (i = 0; i != solver->subsolver_count; ++i)
        IK_DECREF(solver->subsolvers[i]);

    FREE(solver->child_rotations);
    FREE(solver->child_nodes);
    FREE(solver->subsolvers);
}

/* ------------------------------------------------------------------------- */
static int
solver_combine_solve(struct ik_solver* solver_base)
{
    /*
     * All solvers share the same base node, but are themselves unaware of this
     * fact. This meta-solver ensures that each solver receivers the rotation
     * and child node position that it expects, and also ensures that the
     * individual solved rotations are properly recombined into a final rotation.
     */

    int iterations;
    int i;
    union ik_quat rot;
    union ik_quat initial_rot;
    struct ik_solver_combine* solver = (struct ik_solver_combine*)solver_base;
    struct ik_node* node = solver->shared_node;

    /* Store the current averaged rotation, which is required to calculate the
     * isolated rotation of each segment */
    initial_rot = node->rotation;

    /* Normalize translations of all segments to [0, 0, 1]. Each subsolver
     * assumes this to be true. */
    for (i = 0; i != solver->child_node_count; ++i)
    {
        struct ik_node* child = solver->child_nodes[i];
        ik_quat_angle_of(solver->child_rotations[i].f, child->position.f);
        ik_quat_mul_quat_conj(child->rotation.f, solver->child_rotations[i].f);
        ik_vec3_set(child->position.f, 0, 0, ik_vec3_length(child->position.f));
    }

    /* Call solve() on all subsolvers now */
    iterations = 0;
    ik_quat_set(rot.f, 0, 0, 0, 0);
    for (i = 0; i != solver->subsolver_count; ++i)
    {
        struct ik_solver* subsolver = solver->subsolvers[i];

        /* Apply isolated rotation to segment before solve() */
        node->rotation = initial_rot;
        ik_quat_mul_quat(node->rotation.f, solver->child_rotations[i].f);

        iterations += subsolver->impl.solve(subsolver);

        /* Store the solved rotation for later */
        solver->child_rotations[i] = node->rotation;

        /* Average */
        ik_quat_ensure_positive_sign(node->rotation.f);
        ik_quat_add_quat(rot.f, node->rotation.f);
    }

    /* Accumulate isolated rotations of child segments that don't have a solver
     * as well, so their orientation doesn't change */
    for (; i != solver->child_node_count; ++i)
    {
        ik_quat_rmul_quat(initial_rot.f, solver->child_rotations[i].f);
        ik_quat_ensure_positive_sign(solver->child_rotations[i].f);
        ik_quat_add_quat(rot.f, solver->child_rotations[i].f);
    }

    ik_quat_div_scalar(rot.f, solver->child_node_count);
    ik_quat_normalize(rot.f);

    /* Calculate new translations */
    for (i = 0; i != solver->child_node_count; ++i)
    {
        struct ik_node* child = solver->child_nodes[i];
        ik_quat_conj_rmul_quat(rot.f, solver->child_rotations[i].f);
        ik_vec3_rotate_quat(child->position.f, solver->child_rotations[i].f);
        ik_quat_mul_quat(child->rotation.f, solver->child_rotations[i].f);
    }

    /* Apply averaged rotation to shared node */
    node->rotation = rot;

    return iterations;
}

/* ------------------------------------------------------------------------- */
static void
solver_combine_iterate_nodes(const struct ik_solver* solver_base, ik_solver_callback_func cb, int skip_base)
{
    unsigned i;
    struct ik_solver* subsolver;
    const struct ik_solver_combine* solver = (const struct ik_solver_combine*)solver_base;

    /* Last solver in list is the root-most solver, and is the only solver that
     * references the base node. We pass skip_base to it */
    i = solver->subsolver_count - 1;
    subsolver = solver->subsolvers[i];
    subsolver->impl.iterate_nodes(subsolver, cb, skip_base);

    /* Every other solver doesn't reference the base node so explicitely call
     * with skip_base=0 */
    while (i-- > 0)
    {
        subsolver = solver->subsolvers[i];
        subsolver->impl.iterate_nodes(subsolver, cb, 0);
    }
}

/* ------------------------------------------------------------------------- */
static void
solver_combine_iterate_effector_nodes(const struct ik_solver* solver_base, ik_solver_callback_func cb)
{
    int i;
    const struct ik_solver_combine* solver = (const struct ik_solver_combine*)solver_base;

    for (i = 0; i != solver->subsolver_count; ++i)
    {
        struct ik_solver* subsolver = solver->subsolvers[i];
        subsolver->impl.iterate_effector_nodes(subsolver, cb);
    }
}

/* ------------------------------------------------------------------------- */
static void
solver_combine_get_first_segment(const struct ik_solver* solver_base, struct ik_node** base, struct ik_node** tip)
{
    ik_log_printf(IK_ERROR, "solver_combine_get_first_segment() called! This should not happen ever!");
    assert(0);
}

/* ------------------------------------------------------------------------- */
struct ik_solver_interface ik_solver_combine = {
    "combine",
    sizeof(struct ik_solver_combine),
    solver_combine_init,
    solver_combine_deinit,
    solver_combine_solve,
    solver_combine_iterate_nodes,
    solver_combine_iterate_effector_nodes,
    solver_combine_get_first_segment
};

/* ------------------------------------------------------------------------- */
static void
sort_node_list_to_match_subsolvers(struct ik_solver_combine* solver)
{
    int s, n;
    for (s = 0; s != solver->subsolver_count; ++s)
    {
        struct ik_solver* subsolver = solver->subsolvers[s];
        for (n = 0; n != solver->child_node_count; ++n)
        {
            struct ik_node *base, *tip;
            ik_solver_get_first_segment(subsolver, &base, &tip);
            if (solver->child_nodes[n] == tip)
            {
                struct ik_node* tmp = solver->child_nodes[s];
                solver->child_nodes[s] = solver->child_nodes[n];
                solver->child_nodes[n] = tmp;
                break;
            }
        }
    }

#ifdef DEBUG
    for (s = 0; s != solver->subsolver_count; ++s)
    {
        struct ik_node *base, *tip;
        ik_solver_get_first_segment(solver->subsolvers[s], &base, &tip);
        assert(tip == solver->child_nodes[s]);
    }
#endif
}

/* ------------------------------------------------------------------------- */
struct ik_solver*
ik_solver_combine_create(const struct cs_vector* solver_list, struct ik_node* shared_node)
{
    int i;

    struct ik_solver_combine* solver = (struct ik_solver_combine*)
        ik_refcounted_alloc(sizeof *solver, (ik_deinit_func)solver_combine_deinit);
    if (solver == NULL)
        goto alloc_solver_failed;

    /* Allocate array in which we will store the list of subsolvers */
    solver->subsolver_count = vector_count(solver_list);
    solver->subsolvers = MALLOC(sizeof(*solver->subsolvers) * solver->subsolver_count);
    if (solver->subsolvers == NULL)
        goto alloc_subsolvers_failed;

    /*
     * Allocate array in which we will store the list of child nodes of the
     * shared node. Storing this information is necessary because it's possible
     * the user might add or remove nodes from the tree, which would break
     * everything if we used the child node list stored in shared_node
     */
    solver->child_node_count = ik_node_child_count(shared_node);
    solver->child_nodes = MALLOC(sizeof(*solver->child_nodes) * solver->child_node_count);
    if (solver->child_nodes == NULL)
        goto alloc_child_nodes_failed;

    /* Allocate buffer for storing child node rotations temporarily */
    solver->child_rotations = MALLOC(sizeof(*solver->child_rotations) * solver->child_node_count);
    if (solver->child_rotations == NULL)
        goto alloc_child_rotations_failed;

    /* Fill in list of subsolvers */
    for (i = 0; i != solver->subsolver_count; ++i)
    {
        struct ik_solver* subsolver = *(struct ik_solver**)vector_get_element(solver_list, i);
        IK_INCREF(subsolver);
        solver->subsolvers[i] = subsolver;
    }

    /* Fill in list of child nodes */
    for (i = 0; i != solver->child_node_count; ++i)
    {
        struct ik_node* node = ik_node_get_child(shared_node, i);
        IK_INCREF(node);
        solver->child_nodes[i] = node;
    }

    sort_node_list_to_match_subsolvers(solver);

    solver->impl = ik_solver_combine;
    solver->algorithm = NULL;
    solver->shared_node = shared_node;  /* Other solvers are holding a reference to this node so we don't have to */

    return (struct ik_solver*)solver;

    alloc_child_rotations_failed : FREE(solver->child_nodes);
    alloc_child_nodes_failed     : FREE(solver->subsolvers);
    alloc_subsolvers_failed      : ik_refcounted_free((struct ik_refcounted*)solver);
    alloc_solver_failed          : return NULL;
}
