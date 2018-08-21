#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME mat3x3

using namespace ::testing;

const double pi = std::atan(1.0) * 4;

TEST(NAME, from_quat_1)
{
    ik_mat3x3_t m;
    ik_quat_t q;
    ik_quat_set_identity(q.f);
    ik_mat3x3_from_quat(m.f, q.f);

    ikreal_t ex[3] = {1, 0, 0};
    ikreal_t ey[3] = {0, 1, 0};
    ikreal_t ez[3] = {0, 0, 1};
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ex[i], DoubleEq(ex[i]));
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ey[i], DoubleEq(ey[i]));
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ez[i], DoubleEq(ez[i]));
}

TEST(NAME, from_quat_2)
{
    ik_mat3x3_t m;
    ik_quat_t q;
    ik_quat_set_axis_angle(q.f, 0, 1, 0, 45.0*pi/180);
    ik_mat3x3_from_quat(m.f, q.f);

    ikreal_t ex[3] = {1/sqrt(2), 0, 1/sqrt(2)};
    ikreal_t ey[3] = {0, 1, 0};
    ikreal_t ez[3] = {-1/sqrt(2), 0, 1/sqrt(2)};
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ex[i], DoubleEq(ex[i]));
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ey[i], DoubleEq(ey[i]));
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ez[i], DoubleEq(ez[i]));
}

TEST(NAME, from_quat_3)
{
    ik_mat3x3_t m;
    ik_quat_t q;
    ik_quat_set_axis_angle(q.f, 1, 0, 0, 45.0*pi/180);
    ik_mat3x3_from_quat(m.f, q.f);

    ikreal_t ex[3] = {1, 0, 0};
    ikreal_t ey[3] = {0, 1/sqrt(2), -1/sqrt(2)};
    ikreal_t ez[3] = {0, 1/sqrt(2), 1/sqrt(2)};
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ex[i], DoubleEq(ex[i]));
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ey[i], DoubleEq(ey[i]));
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ez[i], DoubleEq(ez[i]));
}

TEST(NAME, from_quat_4)
{
    ik_mat3x3_t m;
    ik_quat_t q;
    ik_quat_set_axis_angle(q.f, 0, 0, 1, 45.0*pi/180);
    ik_mat3x3_from_quat(m.f, q.f);

    ikreal_t ex[3] = {1/sqrt(2), -1/sqrt(2), 0};
    ikreal_t ey[3] = {1/sqrt(2), 1/sqrt(2), 0};
    ikreal_t ez[3] = {0, 0, 1};
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ex[i], DoubleEq(ex[i]));
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ey[i], DoubleEq(ey[i]));
    for (int i = 0; i != 3; ++i)
        EXPECT_THAT(m.ez[i], DoubleEq(ez[i]));
}
