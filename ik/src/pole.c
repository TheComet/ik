#include "ik/ik.h"
#include "ik/log.h"
#include "ik/pole.h"
#include "ik/quat.inl"
#include "ik/vec3.inl"
#include <stddef.h>
#include <string.h>
#include <assert.h>

/* ------------------------------------------------------------------------- */
/* Pole vector constraint implementations */
/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
static void
calculate_roll_generic(const struct ik_pole* pole, ikreal q[4])
{
    ik_quat_set_identity(q);
}

/* ------------------------------------------------------------------------- */
#if 0
/* https://i.stack.imgur.com/lKN6o.jpg */
static void
calculate_roll_blender(ikreal q[4], const struct ik_pole* pole)
{
    union ik_vec3 work, x_axis, z_axis;

    /*
     * Determine "ik axis", which is the vector from the pole node to the
     * effector node.
     */
    ik_vec3_copy(work.f, pole->tip->transform.t.position.f);
    ik_vec3_sub_vec3(work.f, pole->node->transform.t.position.f);

    /*
     * Determine "pole axis", which is perpendicular to "ik axis" and points to
     * the pole position.
     */
    ik_vec3_project_from_vec3(work.f, pole->position.f);
    ik_vec3_sub_vec3(work.f, pole->position.f);

    /* Determine global XZ basis vectors of pole node */
    ik_vec3_set(x_axis.f, 1, 0, 0);
    ik_vec3_set(z_axis.f, 0, 0, 1);
    ik_vec3_rotate(x_axis.f, pole->node->transform.t.rotation.f);
    ik_vec3_rotate(z_axis.f, pole->node->transform.t.rotation.f);

    /* Project the pole axis onto these basis nodes to obtain the projected
     * pole axis */
    ik_vec3_normalize(work.f);
    ik_vec3_project_from_vec3_normalized(x_axis.f, work.f);
    ik_vec3_project_from_vec3_normalized(z_axis.f, work.f);
    ik_vec3_add_vec3(z_axis.f, x_axis.f);  /* z_axis is now projected pole axis */
}
#endif

/* ------------------------------------------------------------------------- *
static void
calculate_roll_maya(ikreal q[4], const struct ik_pole* pole)
{
    ik_quat_set_identity(q);
}*/

/* ------------------------------------------------------------------------- */
/* Pole API */
/* ------------------------------------------------------------------------- */

static void
deinit_pole(struct ik_pole* pole)
{
}

/* ------------------------------------------------------------------------- */
struct ik_pole*
ik_pole_create(void)
{
    struct ik_pole* pole = (struct ik_pole*)
        ik_attachment_alloc(sizeof *pole, (ik_deinit_func)deinit_pole);
    if (pole == NULL)
        return NULL;

    pole->calculate_roll = calculate_roll_generic;
    pole->angle = 0.0;
    ik_vec3_set_zero(pole->position.f);

    return pole;
}

/* ------------------------------------------------------------------------- */
void
ik_pole_set_generic(struct ik_pole* pole)
{
    pole->calculate_roll = calculate_roll_generic;
}

/* ------------------------------------------------------------------------- */
void
ik_pole_set_blender(struct ik_pole* pole)
{
    pole->calculate_roll = calculate_roll_generic;
}

/* ------------------------------------------------------------------------- */
void
ik_pole_set_maya(struct ik_pole* pole)
{
    pole->calculate_roll = calculate_roll_generic;
}
