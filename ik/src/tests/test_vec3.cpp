#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME vec3

using namespace ::testing;

const double pi = std::atan(1.0) * 4;

TEST(NAME, vec3)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    EXPECT_THAT(v.x, DoubleEq(1));
    EXPECT_THAT(v.y, DoubleEq(2));
    EXPECT_THAT(v.z, DoubleEq(3));
}

TEST(NAME, set)
{
    ik_vec3_t v1 = IKAPI.vec3.vec3(1, 2, 3);
    ik_vec3_t v2 = IKAPI.vec3.vec3(4, 5, 6);
    IKAPI.vec3.set(v1.f, v2.f);
    EXPECT_THAT(v1.x, DoubleEq(4));
    EXPECT_THAT(v1.y, DoubleEq(5));
    EXPECT_THAT(v1.z, DoubleEq(6));
}

TEST(NAME, set_zero)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    IKAPI.vec3.set_zero(v.f);
    EXPECT_THAT(v.x, DoubleEq(0));
    EXPECT_THAT(v.y, DoubleEq(0));
    EXPECT_THAT(v.z, DoubleEq(0));
}

TEST(NAME, add_scalar)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    IKAPI.vec3.add_scalar(v.f, 3);
    EXPECT_THAT(v.x, DoubleEq(4));
    EXPECT_THAT(v.y, DoubleEq(5));
    EXPECT_THAT(v.z, DoubleEq(6));
}

TEST(NAME, add_vec3)
{
    ik_vec3_t v1 = IKAPI.vec3.vec3(1, 2, 3);
    ik_vec3_t v2 = IKAPI.vec3.vec3(4, 5, 6);
    IKAPI.vec3.add_vec3(v1.f, v2.f);
    EXPECT_THAT(v1.x, DoubleEq(5));
    EXPECT_THAT(v1.y, DoubleEq(7));
    EXPECT_THAT(v1.z, DoubleEq(9));
}

TEST(NAME, sub_scalar)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    IKAPI.vec3.sub_scalar(v.f, 2);
    EXPECT_THAT(v.x, DoubleEq(-1));
    EXPECT_THAT(v.y, DoubleEq(0));
    EXPECT_THAT(v.z, DoubleEq(1));
}

TEST(NAME, sub_vec3)
{
    ik_vec3_t v1 = IKAPI.vec3.vec3(1, 2, 3);
    ik_vec3_t v2 = IKAPI.vec3.vec3(4, 5, 6);
    IKAPI.vec3.sub_vec3(v1.f, v2.f);
    EXPECT_THAT(v1.x, DoubleEq(-3));
    EXPECT_THAT(v1.y, DoubleEq(-3));
    EXPECT_THAT(v1.z, DoubleEq(-3));
}

TEST(NAME, mul_scalar)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    IKAPI.vec3.mul_scalar(v.f, 3);
    EXPECT_THAT(v.x, DoubleEq(3));
    EXPECT_THAT(v.y, DoubleEq(6));
    EXPECT_THAT(v.z, DoubleEq(9));
}

TEST(NAME, mul_vec3)
{
    ik_vec3_t v1 = IKAPI.vec3.vec3(1, 2, 3);
    ik_vec3_t v2 = IKAPI.vec3.vec3(4, 5, 6);
    IKAPI.vec3.mul_vec3(v1.f, v2.f);
    EXPECT_THAT(v1.x, DoubleEq(4));
    EXPECT_THAT(v1.y, DoubleEq(10));
    EXPECT_THAT(v1.z, DoubleEq(18));
}

TEST(NAME, div_scalar)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    IKAPI.vec3.div_scalar(v.f, 3);
    EXPECT_THAT(v.x, DoubleEq(1.0/3));
    EXPECT_THAT(v.y, DoubleEq(2.0/3));
    EXPECT_THAT(v.z, DoubleEq(3.0/3));
}

TEST(NAME, div_vec3)
{
    ik_vec3_t v1 = IKAPI.vec3.vec3(1, 2, 3);
    ik_vec3_t v2 = IKAPI.vec3.vec3(4, 5, 6);
    IKAPI.vec3.div_vec3(v1.f, v2.f);
    EXPECT_THAT(v1.x, DoubleEq(0.25));
    EXPECT_THAT(v1.y, DoubleEq(0.4));
    EXPECT_THAT(v1.z, DoubleEq(0.5));
}

TEST(NAME, length_squared_positive_numbers)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    EXPECT_THAT(IKAPI.vec3.length_squared(v.f), DoubleEq(14));
}

TEST(NAME, length_squared_negative_numbers)
{
    ik_vec3_t v = IKAPI.vec3.vec3(-1, -2, -3);
    EXPECT_THAT(IKAPI.vec3.length_squared(v.f), DoubleEq(14));
}

TEST(NAME, length_squared_zero)
{
    ik_vec3_t v = IKAPI.vec3.vec3(0, 0, 0);
    EXPECT_THAT(IKAPI.vec3.length_squared(v.f), DoubleEq(0));
}

TEST(NAME, length_positive_numbers)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    EXPECT_THAT(IKAPI.vec3.length(v.f), DoubleEq(sqrt(14)));
}

TEST(NAME, length_negative_numbers)
{
    ik_vec3_t v = IKAPI.vec3.vec3(-1, -2, -3);
    EXPECT_THAT(IKAPI.vec3.length(v.f), DoubleEq(sqrt(14)));
}

TEST(NAME, length_zero)
{
    ik_vec3_t v = IKAPI.vec3.vec3(0, 0, 0);
    EXPECT_THAT(IKAPI.vec3.length(v.f), DoubleEq(0));
}

TEST(NAME, normalize_positive_numbers)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    IKAPI.vec3.normalize(v.f);
    EXPECT_THAT(v.x, DoubleEq(1.0 / sqrt(14)));
    EXPECT_THAT(v.y, DoubleEq(2.0 / sqrt(14)));
    EXPECT_THAT(v.z, DoubleEq(3.0 / sqrt(14)));
}

TEST(NAME, normalize_negative_numbers)
{
    ik_vec3_t v = IKAPI.vec3.vec3(-1, -2, -3);
    IKAPI.vec3.normalize(v.f);
    EXPECT_THAT(v.x, DoubleEq(-1.0 / sqrt(14)));
    EXPECT_THAT(v.y, DoubleEq(-2.0 / sqrt(14)));
    EXPECT_THAT(v.z, DoubleEq(-3.0 / sqrt(14)));
}

TEST(NAME, normalize_zero)
{
    // Due to vectors being used as directions, make the default direction 1,0,0
    ik_vec3_t v = IKAPI.vec3.vec3(0, 0, 0);
    IKAPI.vec3.normalize(v.f);
    EXPECT_THAT(v.x, DoubleEq(1));
    EXPECT_THAT(v.y, DoubleEq(0));
    EXPECT_THAT(v.z, DoubleEq(0));
}

TEST(NAME, dot)
{
    ik_vec3_t v1 = IKAPI.vec3.vec3(1, 2, 3);
    ik_vec3_t v2 = IKAPI.vec3.vec3(4, 5, 6);
    EXPECT_THAT(IKAPI.vec3.dot(v1.f, v2.f), DoubleEq(32));
}

TEST(NAME, cross)
{
    ik_vec3_t v1 = IKAPI.vec3.vec3(1, 2, 3);
    ik_vec3_t v2 = IKAPI.vec3.vec3(4, 5, 6);
    IKAPI.vec3.cross(v1.f, v2.f);
    EXPECT_THAT(v1.x, DoubleEq(-3));
    EXPECT_THAT(v1.y, DoubleEq(6));
    EXPECT_THAT(v1.z, DoubleEq(-3));
}

TEST(NAME, ncross)
{
    ik_vec3_t v1 = IKAPI.vec3.vec3(1, 2, 3);
    ik_vec3_t v2 = IKAPI.vec3.vec3(4, 5, 6);
    IKAPI.vec3.ncross(v1.f, v2.f);
    EXPECT_THAT(v1.x, DoubleEq(3));
    EXPECT_THAT(v1.y, DoubleEq(-6));
    EXPECT_THAT(v1.z, DoubleEq(3));
}

TEST(NAME, rotate_vector_45_degree)
{
    ik_quat_t q;
    ik_vec3_t axis = IKAPI.vec3.vec3(0, 1, 0);
    IKAPI.quat.set_axis_angle(q.f, axis.f, 45.0*pi/180);

    ik_vec3_t v = IKAPI.vec3.vec3(1, 0, 0);
    IKAPI.vec3.rotate(v.f, q.f);

    EXPECT_THAT(v.x, DoubleEq(1/sqrt(2)));
    EXPECT_THAT(v.y, DoubleEq(0));
    EXPECT_THAT(v.z, DoubleEq(-1/sqrt(2)));
}

TEST(NAME, rotate_vector_120_degree_steps)
{
    ik_quat_t q;
    ik_vec3_t axis = IKAPI.vec3.vec3(1, 1, 1);
    ik_vec3_t v = IKAPI.vec3.vec3(1, 0, 0);
    IKAPI.quat.set_axis_angle(q.f, axis.f, 120 * pi / 180);

    IKAPI.vec3.rotate(v.f, q.f);
    EXPECT_THAT(v.x, DoubleNear(0, 1e-15));
    EXPECT_THAT(v.y, DoubleNear(1, 1e-15));
    EXPECT_THAT(v.z, DoubleNear(0, 1e-15));

    IKAPI.vec3.rotate(v.f, q.f);
    EXPECT_THAT(v.x, DoubleNear(0, 1e-15));
    EXPECT_THAT(v.y, DoubleNear(0, 1e-15));
    EXPECT_THAT(v.z, DoubleNear(1, 1e-15));

    IKAPI.vec3.rotate(v.f, q.f);
    EXPECT_THAT(v.x, DoubleNear(1, 1e-15));
    EXPECT_THAT(v.y, DoubleNear(0, 1e-15));
    EXPECT_THAT(v.z, DoubleNear(0, 1e-15));
}

TEST(NAME, rotate_vector_there_and_back)
{
    ik_quat_t q;
    ik_vec3_t axis = IKAPI.vec3.vec3(63, 9679, 34);
    ik_vec3_t v = IKAPI.vec3.vec3(1, 0, 0);
    IKAPI.quat.set_axis_angle(q.f, axis.f, 48.32 * pi / 180);
    IKAPI.vec3.rotate(v.f, q.f);
    IKAPI.quat.conj(q.f);
    IKAPI.vec3.rotate(v.f, q.f);
    EXPECT_THAT(v.x, DoubleNear(1, 1e-15));
    EXPECT_THAT(v.y, DoubleNear(0, 1e-15));
    EXPECT_THAT(v.z, DoubleNear(0, 1e-15));
}

TEST(NAME, project)
{
    ik_vec3_t u = IKAPI.vec3.vec3(2, 2, 2);
    ik_vec3_t v = IKAPI.vec3.vec3(7, 0, 7);
    IKAPI.vec3.project_from_vec3(v.f, u.f);
    EXPECT_THAT(v.x, DoubleNear(7.0 * 28 / 98, 1e-15));
}
