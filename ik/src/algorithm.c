#include "ik/algorithm.h"
#include "ik/log.h"
#include "ik/quat.h"
#include "ik/tree_object.h"
#include "ik/vec3.h"
#include <string.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
static int
is_algorithm_type_valid(const char* type)
{
    uintptr_t len = strlen(type);
    if (len >= sizeof(((struct ik_algorithm*)0)->type))
    {
        ik_log_printf(IK_ERROR, "Algorithm type `%s` is too long.", type);
        return 0;
    }
    if (len == 0)
    {
        ik_log_printf(IK_ERROR, "Empty algorithm type provided.");
        return 0;
    }
    return 1;
}

/* ------------------------------------------------------------------------- */
struct ik_algorithm*
ik_algorithm_create(const char* type)
{
    struct ik_algorithm* alg;

    if (!is_algorithm_type_valid(type))
        return NULL;

    alg = (struct ik_algorithm*)
        ik_attachment_alloc(sizeof *alg, NULL);
    if (alg == NULL)
        return NULL;

    strcpy(alg->type, type);
    alg->tolerance = 0.0;
    alg->max_iterations = 20;
    alg->features = 0;

    return alg;
}

/* ------------------------------------------------------------------------- */
int
ik_algorithm_set_type(struct ik_algorithm* algorithm, const char* type)
{
    if (!is_algorithm_type_valid(type))
        return -1;

    strcpy(algorithm->type, type);
    return 0;
}

/* ------------------------------------------------------------------------- */
struct ik_algorithm*
ik_algorithm_duplicate(const struct ik_algorithm* algorithm)
{
    struct ik_algorithm* dup = (struct ik_algorithm*)
        ik_attachment_alloc(sizeof *dup, NULL);
    if (dup == NULL)
        return NULL;

    strcpy(dup->type, algorithm->type);
    dup->tolerance = algorithm->tolerance;
    dup->max_iterations = algorithm->max_iterations;
    dup->features = algorithm->features;

    return dup;
}

/* ------------------------------------------------------------------------- */
static int
count_algorithms(const struct ik_tree_object* root)
{
    int count = root->algorithm ? 1 : 0;
    TREE_OBJECT_FOR_EACH_CHILD(root, child)
        count += count_algorithms(child);
    TREE_OBJECT_END_EACH
    return count;
}

/* ------------------------------------------------------------------------- */
static void
copy_from_tree(struct ik_algorithm** alg_buf, struct ik_tree_object* dst,
               const struct ik_tree_object* src)
{
    uint32_t i;

    if (src->algorithm)
    {
        struct ik_algorithm* alg = *alg_buf;
        (*alg_buf)++;

        strcpy(alg->type, src->algorithm->type);
        alg->tolerance = src->algorithm->tolerance;
        alg->max_iterations = src->algorithm->max_iterations;
        alg->features = src->algorithm->features;

        ik_tree_object_attach_algorithm(dst, alg);
    }

    assert(ik_tree_object_child_count(src) == ik_tree_object_child_count(dst));
    for (i = 0; i != ik_tree_object_child_count(src); ++i)
    {
        copy_from_tree(alg_buf,
                       ik_tree_object_get_child(dst, i),
                       ik_tree_object_get_child(src, i));
    }
}

/* ------------------------------------------------------------------------- */
int
ik_algorithm_duplicate_from_tree(struct ik_tree_object* dst,
                                 const struct ik_tree_object* src)
{
    int alg_count;
    struct ik_algorithm* alg_buf;

    alg_count = count_algorithms(src);
    if (alg_count == 0)
        return 0;

    alg_buf = (struct ik_algorithm*)
        ik_refcounted_alloc_array(sizeof *alg_buf, NULL, alg_count);
    if (alg_buf == NULL)
        return -1;

    copy_from_tree(&alg_buf, dst, src);
    return 0;
}
