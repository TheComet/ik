#include "ik/ik.h"
#include "ik/memory.h"
#include "ik/node.h"
#include "ik/log.h"
#include "ik/pole.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include <stddef.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
static void
calculate_roll_generic(ikreal_t q[4], const struct ik_pole_t* pole)
{
    ik_quat_set_identity(q);
}

/* ------------------------------------------------------------------------- */
static void
calculate_roll_blender(ikreal_t q[4], const struct ik_pole_t* pole)
{
    struct ik_vec3_t work, x_axis, z_axis;
    struct ik_node_t* root = pole->node;
    while (root->parent)
        root = root->parent;

    /*
     * Determine "ik axis", which is the vector from the pole node to the
     * effector node.
     */
    ik_vec3_copy(work.f, pole->tip->position.f);
    ik_vec3_sub_vec3(work.f, pole->node->position.f);

    /*
     * Determine "pole axis", which is perpendicular to "ik axis" and points to
     * the pole position.
     */
    ik_vec3_project_from_vec3(work.f, pole->position.f);
    ik_vec3_sub_vec3(work.f, pole->position.f);

    /* Determine global XZ basis vectors of pole node */
    ik_vec3_set(x_axis.f, 1, 0, 0);
    ik_vec3_set(z_axis.f, 0, 0, 1);
    ik_vec3_rotate(x_axis.f, pole->node->rotation.f);
    ik_vec3_rotate(z_axis.f, pole->node->rotation.f);

    /* Project the pole axis onto these basis nodes to obtain the projected
     * pole axis */
    ik_vec3_normalize(work.f);
    ik_vec3_project_from_vec3_normalized(x_axis.f, work.f);
    ik_vec3_project_from_vec3_normalized(z_axis.f, work.f);
    ik_vec3_add_vec3(z_axis.f, x_axis.f);  /* z_axis is now projected pole axis */
}

/* ------------------------------------------------------------------------- */
static void
calculate_roll_maya(ikreal_t q[4], const struct ik_pole_t* pole)
{
    ik_quat_set_identity(q);
}

/* ------------------------------------------------------------------------- */
struct ik_pole_t*
ik_pole_create(void)
{
    struct ik_pole_t* pole = MALLOC(sizeof *pole);
    if (pole == NULL)
    {
        ik_log_fatal("Failed to allocate pole: Out of memory");
        return NULL;
    }

    memset(pole, 0, sizeof *pole);

    pole->angle = 0.0;
    pole->calculate_roll = calculate_roll_generic;
    ik_vec3_set_zero(pole->position.f);

    return pole;
}

/* ------------------------------------------------------------------------- */
void
ik_pole_destroy(struct ik_pole_t* pole)
{
    ik_pole_detach(pole);
    FREE(pole);
}

/* ------------------------------------------------------------------------- */
void
ik_pole_set_type(struct ik_pole_t* pole, enum ik_pole_type_e type)
{
    switch (type)
    {
        case IK_GENERIC : pole->calculate_roll = calculate_roll_generic; break;
        case IK_BLENDER : pole->calculate_roll = calculate_roll_blender; break;
        case IK_MAYA    : pole->calculate_roll = calculate_roll_maya;    break;
    }
}

/* ------------------------------------------------------------------------- */
struct ik_pole_t*
ik_pole_duplicate(const struct ik_pole_t* pole)
{
    struct ik_pole_t* new_pole = ik_pole_create();
    if (pole == NULL)
        return NULL;

    new_pole->node = NULL;
    new_pole->tip  = NULL,
    new_pole->angle          = pole->angle;
    new_pole->calculate_roll = pole->calculate_roll;

    return new_pole;
}

/* ------------------------------------------------------------------------- */
ikret_t
ik_pole_attach(struct ik_pole_t* pole, struct ik_node_t* node)
{
    if (node->pole != NULL)
    {
        ik_log_error(
            "You are trying to attach a pole to a node that "
            "already has a pole attached to it. The new pole will not "
            "be attached!"
        );
        return -1;
    }

    /* pole may be attached to another node */
    ik_pole_detach(pole);

    node->pole = pole;
    pole->node = node;

    return 0;
}

/* ------------------------------------------------------------------------- */
void
ik_pole_detach(struct ik_pole_t* pole)
{
    if (pole->node == NULL)
        return;

    pole->node->pole = NULL;
    pole->node = NULL;
}
