#include "ik/algorithm.h"
#include "ik/bone.h"
#include "ik/meta_solvers.h"
#include "ik/solver.h"
#include "ik/subtree.h"
#include "ik/log.h"
#include "cstructures/btree.h"
#include "cstructures/vector.h"
#include <string.h>

static struct cs_vector g_solvers;

#define INVALID_COLOR -1

/* ------------------------------------------------------------------------- */
static void
deinit_solver_base(struct ik_solver* solver)
{
    IK_XDECREF(solver->algorithm);
    IK_XDECREF(solver->root_bone);
}

/* ------------------------------------------------------------------------- */
static void
deinit_solver(struct ik_solver* solver)
{
    solver->impl.deinit((struct ik_solver*)solver);
    deinit_solver_base(solver);
}

/* ------------------------------------------------------------------------- */
void
ik_solver_free(struct ik_solver* solver)
{
    deinit_solver_base(solver);
    ik_refcounted_obj_free((struct ik_refcounted*)solver);
}

/* ------------------------------------------------------------------------- */
struct ik_solver*
ik_solver_alloc(const struct ik_solver_interface* impl,
               struct ik_algorithm* algorithm,
               struct ik_bone* root_bone)
{
    struct ik_solver* solver = (struct ik_solver*)
        ik_refcounted_alloc(impl->size, (ik_deinit_func)deinit_solver);
    if (solver == NULL)
        return NULL;

    solver->impl = *impl;
    solver->algorithm = algorithm;
    solver->root_bone = root_bone;

    IK_XINCREF(algorithm);
    IK_XINCREF(root_bone);

    return solver;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_init_builtin_interfaces()
{
#define X(name) extern const struct ik_solver_interface ik_solver_##name;
    IK_ALGORITHM_LIST
#undef X

    vector_init(&g_solvers, sizeof(struct ik_solver_interface*));

#define X(name) \
    if (ik_solver_register(&ik_solver_##name) != 0) \
        goto register_solver_failed;
    IK_ALGORITHM_LIST
#undef X

    return 0;

    register_solver_failed : vector_deinit(&g_solvers);
    return -1;
}

/* ------------------------------------------------------------------------- */
void
ik_solver_deinit_builtin_interfaces(void)
{
    vector_deinit(&g_solvers);
}

/* ------------------------------------------------------------------------- */
int
ik_solver_register(const struct ik_solver_interface* interface)
{
    VECTOR_FOR_EACH(&g_solvers, struct ik_solver_interface*, p_iface)
        if (strcmp((*p_iface)->name, interface->name) == 0)
        {
            ik_log_printf(IK_ERROR, "Solver with name `%s` already registered!", interface->name);
            return -1;
        }
    VECTOR_END_EACH

    if (vector_push(&g_solvers, &interface) != 0)
    {
        ik_log_out_of_memory("ik_solver_register()");
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_unregister(const struct ik_solver_interface* interface)
{
    VECTOR_FOR_EACH(&g_solvers, struct ik_solver_interface*, p_iface)
        if (strcmp((*p_iface)->name, interface->name) == 0)
        {
            vector_erase_element(&g_solvers, p_iface);
            return 0;
        }
    VECTOR_END_EACH

    ik_log_printf(IK_ERROR, "Solver with name `%s` not found!", interface->name);
    return -1;
}

/* ------------------------------------------------------------------------- */
static int
find_all_effector_bones(struct cs_vector* result, const struct ik_bone* bone)
{
    BONE_FOR_EACH_CHILD(bone, child)
        if (find_all_effector_bones(result, child) != 0)
            return -1;
    BONE_END_EACH

    if (bone->effector != NULL)
        if (vector_push(result, &bone) != 0)
            return -1;

    return 0;
}

/* ------------------------------------------------------------------------- */
static int
color_chain(struct cs_btree* colors,
            const struct ik_bone* root,
            const struct ik_bone* bone,
            int chain_length,
            int* color_counter,
            int* chain_color)
{
    /* If chain joins up with any colored part of the tree, adopt that color */
    if (*chain_color == INVALID_COLOR)
    {
        int* existing_color = btree_find(colors, (cs_btree_key)bone);
        if (existing_color)
            *chain_color = *existing_color;
    }

#define is_chain_end() (                            \
    (bone == root)                               || \
    (ik_bone_get_parent(bone)->effector != NULL) || \
    (chain_length == 1))

    /* Recurse */
    if (!is_chain_end())
    {
        if (color_chain(colors, root, ik_bone_get_parent(bone), chain_length - 1, color_counter, chain_color) != 0)
            return -1;
    }
#undef is_chain_end

    /*
     * End of chain reached. If it did not join with anything, create a new
     * color
     */
    if (*chain_color == INVALID_COLOR)
    {
        *chain_color = *color_counter;
        (*color_counter)++;
    }

    switch (btree_insert_new(colors, (cs_btree_key)bone, chain_color))
    {
        case BTREE_EXISTS:
        case BTREE_OK:
            break;

        default:
            ik_log_out_of_memory("color_chain_recurse()");
            return -1;
    }

    return 0;
}
static int
color_reachable_bones(struct cs_btree* colors,
                      const struct cs_vector* effector_bones,
                      const struct ik_bone* root)
{
    int color_counter = 0;

    /*
     * Iterate the chain of bones starting at each effector bone and ending
     * at the specified chain length of the effector, mark every bone on the
     * way.
     */
    VECTOR_FOR_EACH(effector_bones, const struct ik_bone*, p_effector_bone)
        int chain_color = INVALID_COLOR;
        const struct ik_bone* bone = *p_effector_bone;
        const struct ik_effector* effector = bone->effector;
        if (color_chain(colors, root, bone, (int)effector->chain_length, &color_counter, &chain_color) != 0)
            return -1;
    VECTOR_END_EACH

    return 0;
}

/* ------------------------------------------------------------------------- */
enum solver_create_result
{
    SOLVER_CREATED          =  1,
    SOLVER_NOT_CREATED      =  0,
    SOLVER_CREATION_FAILURE = -1
};
static enum solver_create_result
find_algorithm_and_create_solver(struct ik_solver** solver, struct ik_subtree* subtree, struct ik_bone* root)
{
    const struct ik_bone* bone;
    struct ik_algorithm* algorithm;

    algorithm = NULL;
    for (bone = subtree->root; bone != ik_bone_get_parent(root); bone = ik_bone_get_parent(bone))
        if (bone->algorithm != NULL)
        {
            algorithm = bone->algorithm;
            break;
        }
    if (algorithm == NULL)
    {
        ik_log_printf(IK_WARN, "Found bones that are influenced by end-effectors, but couldn't find an attached algorithm. This subtree will be ignored.");
        return SOLVER_NOT_CREATED;
    }

    /* Find a solver that matches the algorithm name */
    VECTOR_FOR_EACH(&g_solvers, struct ik_solver_interface*, p_iface)
        if (strcmp((*p_iface)->name, algorithm->type) == 0)
        {
            struct ik_solver_interface* iface = *p_iface;
            *solver = ik_solver_alloc(iface, algorithm, root);
            if (*solver == NULL)
            {
                ik_log_out_of_memory("create_solver()");
                return SOLVER_CREATION_FAILURE;
            }

            if ((*solver)->impl.init((struct ik_solver*)(*solver), subtree) != 0)
            {
                ik_solver_free(*solver);
                return SOLVER_CREATION_FAILURE;
            }

            return SOLVER_CREATED;
        }
    VECTOR_END_EACH

    ik_log_printf(IK_ERROR, "Unknown algorithm \"%s\". failed to allocate solver", algorithm->type);
    return SOLVER_CREATION_FAILURE;
}

/* ------------------------------------------------------------------------- */
static int
create_solver_for_each_subtree_recurse(struct cs_vector* solver_list,
                                       struct ik_bone* root,
                                       struct ik_bone* bone,
                                       struct ik_subtree* current_subtree,
                                       int parent_color,
                                       const struct cs_btree* colors_map);
static int
recurse_with_subtree(struct cs_vector* solver_list,
                     struct ik_bone* root,
                     struct ik_bone* bone,
                     struct ik_subtree* current_subtree,
                     int bone_color,
                     const struct cs_btree* colors_map)
{
    if (bone->effector != NULL)
    {
        assert(current_subtree != NULL);
        if (subtree_add_leaf(current_subtree, bone) != 0)
            return -1;
    }

    BONE_FOR_EACH_CHILD(bone, child)
        if (create_solver_for_each_subtree_recurse(solver_list, root, child, current_subtree, bone_color, colors_map) != 0)
            return -1;
    BONE_END_EACH

    return 0;
}
static int
start_new_subtree(struct cs_vector* solver_list,
                  struct ik_bone* root,
                  struct ik_bone* bone,
                  int bone_color,
                  const struct cs_btree* colors_map)
{
    struct ik_solver* solver;
    struct ik_subtree subtree;

    /* Begin new subtree */
    subtree_init(&subtree);
    subtree_set_root(&subtree, bone);

    if (recurse_with_subtree(solver_list, root, bone, &subtree, bone_color, colors_map) != 0)
        goto start_new_subtree_fail;

    /* Maybe there are no more bones that need to be solved */
    if (subtree_leaves(&subtree) == 0)
        goto start_new_subtree_success;

    /* This function can succeed without creating a solver, in which case it
     * returns 0. */
    switch (find_algorithm_and_create_solver(&solver, &subtree, root))
    {
        case SOLVER_NOT_CREATED      : goto start_new_subtree_success;
        case SOLVER_CREATION_FAILURE : goto start_new_subtree_fail;
        case SOLVER_CREATED : {
            if (solver && vector_push(solver_list, &solver) != 0)
                goto start_new_subtree_fail;
        } break;
    }

    start_new_subtree_success : subtree_deinit(&subtree);
    return 0;

    start_new_subtree_fail : subtree_deinit(&subtree);
    return -1;
}
static int
create_solver_for_each_subtree_recurse(struct cs_vector* solver_list,
                                       struct ik_bone* root,
                                       struct ik_bone* bone,
                                       struct ik_subtree* current_subtree,
                                       int parent_color,
                                       const struct cs_btree* colors_map)
{
    const int* bone_color = btree_find(colors_map, (cs_btree_key)bone);
    if (bone_color)
    {
        if(*bone_color != parent_color)
        {
            if (start_new_subtree(solver_list, root, bone, *bone_color, colors_map) != 0)
                return -1;
        }
        else
        {
            if (recurse_with_subtree(solver_list, root, bone, current_subtree, *bone_color, colors_map) != 0)
                return -1;
        }
    }
    else
    {
        if (recurse_with_subtree(solver_list, root, bone, current_subtree, INVALID_COLOR, colors_map) != 0)
            return -1;
    }

    return 0;
}
static int
create_solver_for_each_subtree(struct cs_vector* solver_list,
                               struct ik_bone* root,
                               const struct cs_btree* colors_map)
{
    if (create_solver_for_each_subtree_recurse(solver_list, root, root, NULL, INVALID_COLOR, colors_map) != 0)
    {
        VECTOR_FOR_EACH(solver_list, struct ik_solver*, psolver)
            IK_INCDECREF(*psolver);
        VECTOR_END_EACH
        vector_clear(solver_list);
        return -1;
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
struct ik_solver*
ik_solver_build(struct ik_bone* root)
{
    struct cs_vector effector_bones;
    struct cs_btree bone_colors;
    struct cs_vector solver_list;
    struct ik_solver* solver;

    /* Create a list of all bones that have effectors attached */
    vector_init(&effector_bones, sizeof(struct ik_bone*));
    if (find_all_effector_bones(&effector_bones, root) != 0)
        goto find_effectors_failed;

    /* May not have to do anything if none were found */
    if (vector_count(&effector_bones) == 0)
    {
        ik_log_printf(IK_WARN, "No effectors were found in the tree. No solvers were created.");
        goto find_effectors_failed;
    }

    /* Mark all bones that the effectors can reach */
    btree_init(&bone_colors, sizeof(int));
    if (color_reachable_bones(&bone_colors, &effector_bones, root) != 0)
        goto color_reachable_bones_failed;

    /*
     * It's possible that chain length limits end up isolating parts of the
     * tree, splitting it into a list of "sub-trees" which must be solved
     * in a particular order (root to leaf).
     */
    vector_init(&solver_list, sizeof(struct ik_solver*));
    if (create_solver_for_each_subtree(&solver_list, root, &bone_colors) != 0)
        goto split_into_subtrees_failed;

    if (vector_count(&solver_list) == 0)
    {
        solver = NULL;
        vector_deinit(&solver_list);
    }
    else if (vector_count(&solver_list) == 1)
    {
        solver = *(struct ik_solver**)vector_back(&solver_list);
        vector_deinit(&solver_list);
    }
    else
    {
        vector_reverse(&solver_list);
        solver = ik_solver_group_create(&solver_list);
        if (solver == NULL)
            goto create_meta_solver_failed;
        vector_deinit(&solver_list);
    }

    btree_deinit(&bone_colors);
    vector_deinit(&effector_bones);

    return solver;

    create_meta_solver_failed      :
    split_into_subtrees_failed     : vector_deinit(&solver_list);
    color_reachable_bones_failed   : btree_deinit(&bone_colors);
    find_effectors_failed          : vector_deinit(&effector_bones);
    return NULL;
}

/* ------------------------------------------------------------------------- */
int
ik_solver_solve(struct ik_solver* solver)
{
    return solver->impl.solve(solver);
}
