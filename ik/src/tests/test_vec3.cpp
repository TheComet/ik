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
