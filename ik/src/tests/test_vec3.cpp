#include "gmock/gmock.h"
#include "ik/ik.h"

#define NAME vec3

using namespace ::testing;

TEST(NAME, vec3)
{
    ik_vec3_t v = IKAPI.vec3.vec3(1, 2, 3);
    EXPECT_THAT(v.x, DoubleEq(1));
    EXPECT_THAT(v.y, DoubleEq(2));
    EXPECT_THAT(v.z, DoubleEq(3));
}

TEST(NAME, rotate_vector_45_degree)
{
    ik_quat_t q;
    ik_vec3_t axis = IKAPI.vec3.vec3(0, 1, 0);
    IKAPI.quat.set_axis_angle(q.f, axis.f, 45.0*M_PI/180);

    ik_vec3_t v = IKAPI.vec3.vec3(1, 0, 0);
    IKAPI.vec3.rotate(v.f, q.f);

    EXPECT_THAT(v.x, DoubleEq(1/sqrt(2)));
    EXPECT_THAT(v.y, DoubleEq(0));
    EXPECT_THAT(v.z, DoubleEq(-1/sqrt(2)));
}
