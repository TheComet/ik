#include "gmock/gmock.h"
#include "ik/quat.inl"

#define _USE_MATH_DEFINES
#include <math.h>

#define NAME quat

using namespace ::testing;

#define EXPECT_QUAT_EQ(quat, qx, qy, qz, qw) do {                             \
        EXPECT_THAT(quat.q.x, DoubleEq(qx));                                  \
        EXPECT_THAT(quat.q.y, DoubleEq(qy));                                  \
        EXPECT_THAT(quat.q.z, DoubleEq(qz));                                  \
        EXPECT_THAT(quat.q.w, DoubleEq(qw));                                  \
    } while(0)

#define EXPECT_QUAT_NEAR(quat, qx, qy, qz, qw, epsilon) do {                  \
        EXPECT_THAT(quat.q.x, DoubleNear(qx, epsilon));                       \
        EXPECT_THAT(quat.q.y, DoubleNear(qy, epsilon));                       \
        EXPECT_THAT(quat.q.z, DoubleNear(qz, epsilon));                       \
        EXPECT_THAT(quat.q.w, DoubleNear(qw, epsilon));                       \
    } while(0)

TEST(NAME, copy)
{
    union ik_quat q;
    ikreal data1[4] = {1, 2, 3, 4};
    ikreal data2[4] = {5, 6, 7, 8};

    ik_quat_copy(q.f, data1);
    EXPECT_QUAT_EQ(q, 1, 2, 3, 4);

    ik_quat_copy(q.f, data2);
    EXPECT_QUAT_EQ(q, 5, 6, 7, 8);
}

TEST(NAME, set)
{
    union ik_quat q;

    ik_quat_set(q.f, 1, 2, 3, 4);
    EXPECT_QUAT_EQ(q, 1, 2, 3, 4);

    ik_quat_set(q.f, 5, 6, 7, 8);
    EXPECT_QUAT_EQ(q, 5, 6, 7, 8);
}

TEST(NAME, set_identify)
{
    union ik_quat q;
    ik_quat_set_identity(q.f);
    EXPECT_QUAT_EQ(q, 0, 0, 0, 1);
}

TEST(NAME, set_axis_angle)
{
    union ik_quat q;
    const ikreal epsilon = 1e-7;

    ik_quat_set_axis_angle(q.f, 4, 0, 0, M_PI/2);
    EXPECT_QUAT_NEAR(q, 1/sqrt(2), 0, 0, 1/sqrt(2), epsilon);

    ik_quat_set_axis_angle(q.f, 0, 4, 0, M_PI/2);
    EXPECT_QUAT_NEAR(q, 0, 1/sqrt(2), 0, 1/sqrt(2), epsilon);

    ik_quat_set_axis_angle(q.f, 0, 0, 4, M_PI/2);
    EXPECT_QUAT_NEAR(q, 0, 0, 1/sqrt(2), 1/sqrt(2), epsilon);
}

TEST(NAME, add)
{
    union ik_quat q1, q2;

    ik_quat_set(q1.f, 1, 2, 3, 4);
    ik_quat_set(q2.f, 5, 6, 7, 8);
    ik_quat_add_quat(q1.f, q2.f);

    EXPECT_QUAT_EQ(q1, 6, 8, 10, 12);
    EXPECT_QUAT_EQ(q2, 5, 6, 7, 8);
}

TEST(NAME, mag)
{
    union ik_quat q;

    ik_quat_set(q.f, 1, 2, 3, 4);
    EXPECT_THAT(ik_quat_mag(q.f), DoubleEq(sqrt(1*1 + 2*2 + 3*3 + 4*4)));

    ik_quat_set(q.f, 1, -2, -3, 4);
    EXPECT_THAT(ik_quat_mag(q.f), DoubleEq(sqrt(1*1 + 2*2 + 3*3 + 4*4)));
}

TEST(NAME, conj)
{
    union ik_quat q;

    ik_quat_set(q.f, 1, 2, 3, 4);
    ik_quat_conj(q.f);
    EXPECT_QUAT_EQ(q, -1, -2, -3, 4);

    ik_quat_set(q.f, 1, -2, 3, -4);
    ik_quat_conj(q.f);
    EXPECT_QUAT_EQ(q, -1, 2, -3, -4);
}

TEST(NAME, negate)
{
    union ik_quat q;

    ik_quat_set(q.f, 1, 2, 3, 4);
    ik_quat_negate(q.f);
    EXPECT_QUAT_EQ(q, -1, -2, -3, -4);

    ik_quat_set(q.f, 1, -2, 3, -4);
    ik_quat_negate(q.f);
    EXPECT_QUAT_EQ(q, -1, 2, -3, 4);
}

TEST(NAME, invert)
{
    union ik_quat q;
    const ikreal epsilon = 1e-7;

    ik_quat_set(q.f, 1, 2, 3, 4);
    ik_quat_invert(q.f);
    EXPECT_QUAT_NEAR(q, -1.0/(1*1+2*2+3*3+4*4), -2.0/(1*1+2*2+3*3+4*4), -3.0/(1*1+2*2+3*3+4*4), 4.0/(1*1+2*2+3*3+4*4), epsilon);

    ik_quat_set(q.f, 1, -2, 3, -4);
    ik_quat_invert(q.f);
    EXPECT_QUAT_NEAR(q, -1.0/(1*1+2*2+3*3+4*4), 2.0/(1*1+2*2+3*3+4*4), -3.0/(1*1+2*2+3*3+4*4), -4.0/(1*1+2*2+3*3+4*4), epsilon);
}

TEST(NAME, normalize)
{
    union ik_quat q;
    const ikreal epsilon = 1e-7;

    ik_quat_set(q.f, 1, 2, 3, 4);
    ik_quat_normalize(q.f);
    EXPECT_QUAT_NEAR(q, 1.0/sqrt(1*1+2*2+3*3+4*4), 2.0/sqrt(1*1+2*2+3*3+4*4), 3.0/sqrt(1*1+2*2+3*3+4*4), 4.0/sqrt(1*1+2*2+3*3+4*4), epsilon);

    ik_quat_set(q.f, 1, -2, 3, -4);
    ik_quat_normalize(q.f);
    EXPECT_QUAT_NEAR(q, 1.0/sqrt(1*1+2*2+3*3+4*4), -2.0/sqrt(1*1+2*2+3*3+4*4), 3.0/sqrt(1*1+2*2+3*3+4*4), -4.0/sqrt(1*1+2*2+3*3+4*4), epsilon);
}

TEST(NAME, mul)
{
    union ik_quat q1, q2;
    ik_quat_set(q1.f, 1.0/sqrt(2), 0, 0, 1.0/sqrt(2));
    ik_quat_set(q2.f, 0, 1.0/sqrt(2), 0, 1.0/sqrt(2));
    ik_quat_mul_quat(q1.f, q2.f);
    EXPECT_QUAT_NEAR(q1, 0.5, 0.5, 0.5, 0.5, 1e-7);
}

TEST(NAME, nmul)
{

}

TEST(NAME, mul_conj)
{

}

TEST(NAME, nmul_conj)
{

}

TEST(NAME, mul_no_normalize)
{

}


TEST(NAME, nmul_no_normalize)
{

}


TEST(NAME, mul_conj_no_normalize)
{

}

TEST(NAME, nmul_conj_no_normalize)
{

}

TEST(NAME, mul_scalar)
{

}

TEST(NAME, div_scalar)
{

}

TEST(NAME, dot)
{
    union ik_quat q1, q2;
    ik_quat_set(q1.f, 1, 2, 3, 4);
    ik_quat_set(q2.f, 5, 6, 7, 8);
    EXPECT_THAT(ik_quat_dot(q1.f, q2.f), DoubleEq(1*5 + 2*6 + 3*7 + 4*8));
}

TEST(NAME, ensure_positive_sign)
{
    union ik_quat q;

    ik_quat_set(q.f, 1, 2, 3, 4);
    ik_quat_ensure_positive_sign(q.f);
    EXPECT_QUAT_EQ(q, 1, 2, 3, 4);

    ik_quat_set(q.f, 1, 2, 3, -4);
    ik_quat_ensure_positive_sign(q.f);
    EXPECT_QUAT_EQ(q, -1, -2, -3, 4);
}

TEST(NAME, angle_between)
{

}

TEST(NAME, angle_of)
{
    union ik_quat q;
    union ik_vec3 v;

    ik_vec3_set(v.f, 0, 1, 0);
    ik_quat_angle_of(q.f, v.f);
    EXPECT_QUAT_EQ(q, -1/sqrt(2), 0, 0, 1/sqrt(2));

    ik_vec3_set(v.f, 1, 0, 0);
    ik_quat_angle_of(q.f, v.f);
    EXPECT_QUAT_EQ(q, 0, 1/sqrt(2), 0, 1/sqrt(2));
}

TEST(NAME, angle_between_no_normalize)
{

}

TEST(NAME, mul_angle_between)
{

}

TEST(NAME, mul_angle_of)
{

}
