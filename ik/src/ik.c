#include "ik/ik.h"

static uintptr_t g_guid = 0;
static uintptr_t
ik_guid(void)
{
    return g_guid++;
}

static uintptr_t
ik_to_uid(const void* p)
{
    return (uintptr_t)p;
}

static const void*
ik_to_ptr(uintptr_t uid)
{
    return (const void*)uid;
}

struct ik_api_t IKAPI = {
    ik_init,
    ik_deinit,
    ik_guid,
    ik_to_uid,
    ik_to_ptr,
    /*{
        ik_constraint_create,
        ik_constraint_destroy,
        ik_constraint_duplicate,
        ik_constraint_set_type,
        ik_constraint_set_custom,
        ik_constraint_attach,
        ik_constraint_detach,
#define X(arg) IK_CONSTRAINT_##arg,
        IK_CONSTRAINTS_LIST
#undef X
    },*/
    {
#define X(arg, value) value,
        IK_EFFECTOR_FEATURES_LIST
#undef X
    },
    {
        ik_info_author,
        ik_info_version,
        ik_info_build_number,
        ik_info_host,
        ik_info_date,
        ik_info_commit,
        ik_info_compiler,
        ik_info_cmake,
        ik_info_all
    },
    {
        ik_log_init,
        ik_log_deinit,
        ik_log_severity,
        ik_log_timestamps,
        ik_log_prefix,
        ik_log_devel,
        ik_log_info,
        ik_log_warning,
        ik_log_error,
        ik_log_fatal,
#define X(arg) IK_LOG_##arg,
        IK_LOG_SEVERITY_LIST
#undef X
    },
    {
        ik_mat3x3_from_quat
    },
    {
        ik_node_create,
        ik_node_construct,
        ik_node_destruct,
        ik_node_destroy,
        ik_node_destruct_recursive,
        ik_node_destroy_recursive,
        ik_node_create_child,
        ik_node_link,
        ik_node_unlink,
        ik_node_child_count,
        ik_node_find_child,

        ik_node_create_effector,
        ik_node_attach_effector,
        ik_node_release_effector,
        ik_node_take_effector,
        ik_node_get_effector,

        ik_node_create_constraint,
        ik_node_attach_constraint,
        ik_node_release_constraint,
        ik_node_take_constraint,
        ik_node_get_constraint,
/*
        ik_node_create_pole,
        ik_node_attach_pole,
        ik_node_release_pole,
        ik_node_take_pole,
        ik_node_get_pole,*/

        ik_node_set_position,
        ik_node_get_position,
        ik_node_set_rotation,
        ik_node_get_rotation,
        ik_node_set_rotation_weight,
        ik_node_get_rotation_weight,
        ik_node_set_mass,
        ik_node_get_mass,
        ik_node_get_distance_to_parent,
        ik_node_get_user_data,
        ik_node_get_uid
    },/*
    {
        ik_pole_create,
        ik_pole_destroy,
        ik_pole_set_type,
        ik_pole_duplicate,
        ik_pole_attach,
        ik_pole_detach,
#define X(arg) IK_POLE_##arg,
        IK_POLE_TYPE_LIST
#undef X
    },*/
    {
        ik_solver_create,
        ik_solver_destroy,
        ik_solver_construct,
        ik_solver_destruct,
        ik_solver_prepare,
        ik_solver_update_translations,
        ik_solver_solve,
        ik_solver_iterate_nodes,
        ik_solver_get_max_iterations,
        ik_solver_set_max_iterations,
        ik_solver_get_tolerance,
        ik_solver_set_tolerance,
        ik_solver_get_features,
        ik_solver_enable_features,
        ik_solver_disable_features,
        ik_solver_is_feature_enabled,
#define X(arg) IK_SOLVER_##arg,
        IK_SOLVER_ALGORITHM_LIST
#undef X
#define X(arg, value) IK_SOLVER_##arg,
        IK_SOLVER_FEATURES_LIST
#undef X
    },
    {
        ik_quat_copy,
        ik_quat_set,
        ik_quat_set_identity,
        ik_quat_set_axis_angle,
        ik_quat_add_quat,
        ik_quat_mag,
        ik_quat_conj,
        ik_quat_negate,
        ik_quat_invert,
        ik_quat_normalize,
        ik_quat_mul_quat,
        ik_quat_nmul_quat,
        ik_quat_mul_no_normalize,
        ik_quat_nmul_no_normalize,
        ik_quat_mul_scalar,
        ik_quat_div_scalar,
        ik_quat_dot,
        ik_quat_ensure_positive_sign,
        ik_quat_angle,
        ik_quat_angle_no_normalize,
        ik_quat_print
    },
    {
        ik_tests_run
    },/*
    {
        ik_transform_node,
#define X(arg, value) value,
        IK_TRANSFORM_MODE_LIST
#undef X
    },*/
    {
        ik_vec3_copy,
        ik_vec3_set,
        ik_vec3_set_zero,
        ik_vec3_add_scalar,
        ik_vec3_add_vec3,
        ik_vec3_sub_scalar,
        ik_vec3_sub_vec3,
        ik_vec3_mul_scalar,
        ik_vec3_mul_vec3,
        ik_vec3_div_scalar,
        ik_vec3_div_vec3,
        ik_vec3_length_squared,
        ik_vec3_length,
        ik_vec3_normalize,
        ik_vec3_dot,
        ik_vec3_cross,
        ik_vec3_ncross,
        ik_vec3_rotate,
        ik_vec3_nrotate,
        ik_vec3_project_from_vec3,
        ik_vec3_project_from_vec3_normalized,
        ik_vec3_project_onto_plane
    }
};
