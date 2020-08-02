#include "gmock/gmock.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"

#define NAME vec3

using namespace ::testing;

const double pi = std::atan(1.0) * 4;

TEST(NAME, vec3)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);
    EXPECT_THAT(v.v.x, DoubleEq(1));
    EXPECT_THAT(v.v.y, DoubleEq(2));
    EXPECT_THAT(v.v.z, DoubleEq(3));
}

TEST(NAME, copy)
{
    union ik_vec3 v1;
    ik_vec3_set(v1.f, 1, 2, 3);
    union ik_vec3 v2;
    ik_vec3_set(v2.f, 4, 5, 6);
    ik_vec3_copy(v1.f, v2.f);
    EXPECT_THAT(v1.v.x, DoubleEq(4));
    EXPECT_THAT(v1.v.y, DoubleEq(5));
    EXPECT_THAT(v1.v.z, DoubleEq(6));
}

TEST(NAME, set_zero)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);
    ik_vec3_set_zero(v.f);
    EXPECT_THAT(v.v.x, DoubleEq(0));
    EXPECT_THAT(v.v.y, DoubleEq(0));
    EXPECT_THAT(v.v.z, DoubleEq(0));
}

TEST(NAME, add_scalar)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);
    ik_vec3_add_scalar(v.f, 3);
    EXPECT_THAT(v.v.x, DoubleEq(4));
    EXPECT_THAT(v.v.y, DoubleEq(5));
    EXPECT_THAT(v.v.z, DoubleEq(6));
}

TEST(NAME, add_vec3)
{
    union ik_vec3 v1;
    ik_vec3_set(v1.f, 1, 2, 3);
    union ik_vec3 v2;
    ik_vec3_set(v2.f, 4, 5, 6);
    ik_vec3_add_vec3(v1.f, v2.f);
    EXPECT_THAT(v1.v.x, DoubleEq(5));
    EXPECT_THAT(v1.v.y, DoubleEq(7));
    EXPECT_THAT(v1.v.z, DoubleEq(9));
}

TEST(NAME, sub_scalar)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);
    ik_vec3_sub_scalar(v.f, 2);
    EXPECT_THAT(v.v.x, DoubleEq(-1));
    EXPECT_THAT(v.v.y, DoubleEq(0));
    EXPECT_THAT(v.v.z, DoubleEq(1));
}

TEST(NAME, sub_vec3)
{
    union ik_vec3 v1;
    ik_vec3_set(v1.f, 1, 2, 3);
    union ik_vec3 v2;
    ik_vec3_set(v2.f, 4, 5, 6);
    ik_vec3_sub_vec3(v1.f, v2.f);
    EXPECT_THAT(v1.v.x, DoubleEq(-3));
    EXPECT_THAT(v1.v.y, DoubleEq(-3));
    EXPECT_THAT(v1.v.z, DoubleEq(-3));
}

TEST(NAME, mul_scalar)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);
    ik_vec3_mul_scalar(v.f, 3);
    EXPECT_THAT(v.v.x, DoubleEq(3));
    EXPECT_THAT(v.v.y, DoubleEq(6));
    EXPECT_THAT(v.v.z, DoubleEq(9));
}

TEST(NAME, mul_vec3)
{
    union ik_vec3 v1;
    ik_vec3_set(v1.f, 1, 2, 3);
    union ik_vec3 v2;
    ik_vec3_set(v2.f, 4, 5, 6);
    ik_vec3_mul_vec3(v1.f, v2.f);
    EXPECT_THAT(v1.v.x, DoubleEq(4));
    EXPECT_THAT(v1.v.y, DoubleEq(10));
    EXPECT_THAT(v1.v.z, DoubleEq(18));
}

TEST(NAME, div_scalar)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);
    ik_vec3_div_scalar(v.f, 3);
    EXPECT_THAT(v.v.x, DoubleEq(1.0/3));
    EXPECT_THAT(v.v.y, DoubleEq(2.0/3));
    EXPECT_THAT(v.v.z, DoubleEq(3.0/3));
}

TEST(NAME, div_vec3)
{
    union ik_vec3 v1;
    ik_vec3_set(v1.f, 1, 2, 3);
    union ik_vec3 v2;
    ik_vec3_set(v2.f, 4, 5, 6);
    ik_vec3_div_vec3(v1.f, v2.f);
    EXPECT_THAT(v1.v.x, DoubleEq(0.25));
    EXPECT_THAT(v1.v.y, DoubleEq(0.4));
    EXPECT_THAT(v1.v.z, DoubleEq(0.5));
}

TEST(NAME, length_squared_positive_numbers)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);
    EXPECT_THAT(ik_vec3_length_squared(v.f), DoubleEq(14));
}

TEST(NAME, length_squared_negative_numbers)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, -1, -2, -3);
    EXPECT_THAT(ik_vec3_length_squared(v.f), DoubleEq(14));
}

TEST(NAME, length_squared_zero)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 0, 0, 0);
    EXPECT_THAT(ik_vec3_length_squared(v.f), DoubleEq(0));
}

TEST(NAME, length_positive_numbers)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);
    EXPECT_THAT(ik_vec3_length(v.f), DoubleEq(sqrt(14)));
}

TEST(NAME, length_negative_numbers)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, -1, -2, -3);
    EXPECT_THAT(ik_vec3_length(v.f), DoubleEq(sqrt(14)));
}

TEST(NAME, length_zero)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 0, 0, 0);
    EXPECT_THAT(ik_vec3_length(v.f), DoubleEq(0));
}

TEST(NAME, normalize_positive_numbers)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);
    EXPECT_THAT(ik_vec3_normalize(v.f), Eq(1));
    EXPECT_THAT(v.v.x, DoubleEq(1.0 / sqrt(14)));
    EXPECT_THAT(v.v.y, DoubleEq(2.0 / sqrt(14)));
    EXPECT_THAT(v.v.z, DoubleEq(3.0 / sqrt(14)));
}

TEST(NAME, normalize_negative_numbers)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, -1, -2, -3);
    EXPECT_THAT(ik_vec3_normalize(v.f), Eq(1));
    EXPECT_THAT(v.v.x, DoubleEq(-1.0 / sqrt(14)));
    EXPECT_THAT(v.v.y, DoubleEq(-2.0 / sqrt(14)));
    EXPECT_THAT(v.v.z, DoubleEq(-3.0 / sqrt(14)));
}

TEST(NAME, normalize_zero)
{
    union ik_vec3 v;
    ik_vec3_set(v.f, 0, 0, 0);
    EXPECT_THAT(ik_vec3_normalize(v.f), Eq(0));
    EXPECT_THAT(v.v.x, DoubleEq(0));
    EXPECT_THAT(v.v.y, DoubleEq(0));
    EXPECT_THAT(v.v.z, DoubleEq(0));
}

TEST(NAME, dot)
{
    union ik_vec3 v1;
    ik_vec3_set(v1.f, 1, 2, 3);
    union ik_vec3 v2;
    ik_vec3_set(v2.f, 4, 5, 6);
    EXPECT_THAT(ik_vec3_dot(v1.f, v2.f), DoubleEq(32));
}

TEST(NAME, cross)
{
    union ik_vec3 v1;
    ik_vec3_set(v1.f, 1, 2, 3);
    union ik_vec3 v2;
    ik_vec3_set(v2.f, 4, 5, 6);
    ik_vec3_cross(v1.f, v2.f);
    EXPECT_THAT(v1.v.x, DoubleEq(-3));
    EXPECT_THAT(v1.v.y, DoubleEq(6));
    EXPECT_THAT(v1.v.z, DoubleEq(-3));
}

TEST(NAME, ncross)
{
    union ik_vec3 v1;
    ik_vec3_set(v1.f, 1, 2, 3);
    union ik_vec3 v2;
    ik_vec3_set(v2.f, 4, 5, 6);
    ik_vec3_ncross(v1.f, v2.f);
    EXPECT_THAT(v1.v.x, DoubleEq(3));
    EXPECT_THAT(v1.v.y, DoubleEq(-6));
    EXPECT_THAT(v1.v.z, DoubleEq(3));
}

TEST(NAME, rotate_vector_90_x_axis)
{
    // By convention, looking in a positive direction (in this case left to right)
    // a positive rotation about 1, 0, 0 should cause a counter-clockwise
    // rotation

    union ik_quat q;
    ik_quat_set_axis_angle(q.f, 1, 0, 0, 90.0*pi/180);

    union ik_vec3 v;
    ik_vec3_set(v.f, 0, 0, 1);
    ik_vec3_rotate_quat(v.f, q.f);

    EXPECT_THAT(v.v.x, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(-1, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(0, 1e-7));
}

TEST(NAME, rotate_vector_90_y_axis)
{
    // By convention, looking in a positive direction (in this case bottom to top)
    // a positive rotation about 0, 1, 0 should cause a counter-clockwise
    // rotation

    union ik_quat q;
    ik_quat_set_axis_angle(q.f, 0, 1, 0, 90.0*pi/180);

    union ik_vec3 v;
    ik_vec3_set(v.f, 0, 0, 1);
    ik_vec3_rotate_quat(v.f, q.f);

    EXPECT_THAT(v.v.x, DoubleNear(1, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(0, 1e-7));
}

TEST(NAME, rotate_vector_90_z_axis)
{
    // By convention, looking in a positive direction (in this case back to front)
    // a positive rotation about 0, 0, 1 should cause a counter-clockwise
    // rotation

    union ik_quat q;
    ik_quat_set_axis_angle(q.f, 0, 0, 1, 90.0*pi/180);

    union ik_vec3 v;
    ik_vec3_set(v.f, 0, 1, 0);
    ik_vec3_rotate_quat(v.f, q.f);

    EXPECT_THAT(v.v.x, DoubleNear(-1, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(0, 1e-7));
}

TEST(NAME, rotate_vector_45_degree)
{
    union ik_quat q;
    ik_quat_set_axis_angle(q.f, 0, 1, 0, 45.0*pi/180);

    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 0, 0);
    ik_vec3_rotate_quat(v.f, q.f);

    EXPECT_THAT(v.v.x, DoubleEq(1/sqrt(2)));
    EXPECT_THAT(v.v.y, DoubleEq(0));
    EXPECT_THAT(v.v.z, DoubleEq(-1/sqrt(2)));
}

TEST(NAME, rotate_vector_120_degree_steps)
{
    union ik_quat q;
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 0, 0);
    ik_quat_set_axis_angle(q.f, 1, 1, 1, 120 * pi / 180);

    ik_vec3_rotate_quat(v.f, q.f);
    EXPECT_THAT(v.v.x, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(1, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(0, 1e-7));

    ik_vec3_rotate_quat(v.f, q.f);
    EXPECT_THAT(v.v.x, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(1, 1e-7));

    ik_vec3_rotate_quat(v.f, q.f);
    EXPECT_THAT(v.v.x, DoubleNear(1, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(0, 1e-7));
}

TEST(NAME, rotate_vector_there_and_back)
{
    union ik_quat q;
    union ik_vec3 v;
    ik_vec3_set(v.f, 3, 7, 4);
    ik_quat_set_axis_angle(q.f, 63, 9679, 34, 48.32 * pi / 180);
    ik_vec3_rotate_quat(v.f, q.f);
    ik_quat_conj(q.f);
    ik_vec3_rotate_quat(v.f, q.f);
    EXPECT_THAT(v.v.x, DoubleNear(3, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(7, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(4, 1e-7));
}

TEST(NAME, nrotate_vector_45_degree)
{
    union ik_quat q;
    ik_quat_set_axis_angle(q.f, 0, 1, 0, 45.0*pi/180);

    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 0, 0);
    ik_vec3_rotate_quat_conj(v.f, q.f);

    EXPECT_THAT(v.v.x, DoubleEq(1/sqrt(2)));
    EXPECT_THAT(v.v.y, DoubleEq(0));
    EXPECT_THAT(v.v.z, DoubleEq(1/sqrt(2)));
}

TEST(NAME, nrotate_vector_120_degree_steps)
{
    union ik_quat q;
    union ik_vec3 v;
    ik_vec3_set(v.f, 1, 0, 0);
    ik_quat_set_axis_angle(q.f, 1, 1, 1, 120 * pi / 180);

    ik_vec3_rotate_quat_conj(v.f, q.f);
    EXPECT_THAT(v.v.x, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(1, 1e-7));

    ik_vec3_rotate_quat_conj(v.f, q.f);
    EXPECT_THAT(v.v.x, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(1, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(0, 1e-7));

    ik_vec3_rotate_quat_conj(v.f, q.f);
    EXPECT_THAT(v.v.x, DoubleNear(1, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(0, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(0, 1e-7));
}

TEST(NAME, nrotate_vector_there_and_back)
{
    union ik_quat q;
    union ik_vec3 v;
    ik_vec3_set(v.f, 3, 7, 4);
    ik_quat_set_axis_angle(q.f, 63, 9679, 34, 48.32 * pi / 180);
    ik_vec3_rotate_quat_conj(v.f, q.f);
    ik_quat_conj(q.f);
    ik_vec3_rotate_quat_conj(v.f, q.f);
    EXPECT_THAT(v.v.x, DoubleNear(3, 1e-7));
    EXPECT_THAT(v.v.y, DoubleNear(7, 1e-7));
    EXPECT_THAT(v.v.z, DoubleNear(4, 1e-7));
}


TEST(NAME, project)
{
    union ik_vec3 u;
    ik_vec3_set(u.f, 2, 2, 2);
    union ik_vec3 v;
    ik_vec3_set(v.f, 7, 0, 7);
    ik_vec3_project_from_vec3(v.f, u.f);
    EXPECT_THAT(v.v.x, DoubleNear(7.0 * 28 / 98, 1e-7));
}
