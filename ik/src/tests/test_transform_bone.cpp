#include <gmock/gmock.h>
#include "ik/bone.h"
#include "ik/cpputils.hpp"
#include "ik/transform.h"
#include "ik/vec3.inl"

#define NAME transform_bone

#define EXPECT_VEC3_EQ(vec, vx, vy, vz) do {                                  \
        EXPECT_THAT(vec.v.x, DoubleEq(vx));                                   \
        EXPECT_THAT(vec.v.y, DoubleEq(vy));                                   \
        EXPECT_THAT(vec.v.z, DoubleEq(vz));                                   \
    } while(0)

#define EXPECT_VEC3_NEAR(vec, vx, vy, vz, epsilon) do {                       \
        EXPECT_THAT(vec.v.x, DoubleNear(vx, epsilon));                        \
        EXPECT_THAT(vec.v.y, DoubleNear(vy, epsilon));                        \
        EXPECT_THAT(vec.v.z, DoubleNear(vz, epsilon));                        \
    } while(0)

using namespace testing;

TEST(NAME, src_and_dst_are_same)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    b0->length = 2.0;

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_g2l(v.f, b0, b0);
    EXPECT_VEC3_EQ(v, 1, 2, 3);

    ik_transform_bone_pos_l2g(v.f, b0, b0);
    EXPECT_VEC3_EQ(v, 1, 2, 3);
}

TEST(NAME, g2l_2_bones_with_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;
    b1->length = 3.0;
    b2->length = 4.0;

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_g2l(v.f, b0, b2); // from space of b0 to space of b2
    EXPECT_VEC3_NEAR(v, 1, 2, -2, 1e-7);
}

TEST(NAME, g2l_3_bones_without_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;
    b1->length = 3.0;
    b2->length = 4.0;

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_g2l(v.f, ik_bone_get_parent(b0), b2); // global space to space of b2
    EXPECT_VEC3_NEAR(v, 1, 2, -2, 1e-7);
}

TEST(NAME, g2l_2_disconnected_bones_with_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;  ik_vec3_set(b0->position.f, 1, 1, 1);
    b1->length = 3.0;  ik_vec3_set(b1->position.f, 2, 2, 2);
    b2->length = 4.0;  ik_vec3_set(b2->position.f, 3, 3, 3);

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_g2l(v.f, b0, b2); // from space of b0 to space of b2
    EXPECT_VEC3_NEAR(v, -4, -3, -7, 1e-7);
}

TEST(NAME, g2l_3_disconnected_bones_without_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;  ik_vec3_set(b0->position.f, 1, 1, 1);
    b1->length = 3.0;  ik_vec3_set(b1->position.f, 2, 2, 2);
    b2->length = 4.0;  ik_vec3_set(b2->position.f, 3, 3, 3);

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_g2l(v.f, ik_bone_get_parent(b0), b2); // global space to space of b2
    EXPECT_VEC3_NEAR(v, -5, -4, -8, 1e-7);
}

TEST(NAME, g2l_2_disconnected_bones_with_rotation_and_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;  ik_vec3_set(b0->position.f, 1, 1, 1);  ik_quat_set_axis_angle(b0->rotation.f, 1, 0, 0, M_PI/4);
    b1->length = 3.0;  ik_vec3_set(b1->position.f, 2, 2, 2);  ik_quat_set_axis_angle(b1->rotation.f, 0, 1, 0, M_PI/4);
    b2->length = 4.0;  ik_vec3_set(b2->position.f, 3, 3, 3);  ik_quat_set_axis_angle(b2->rotation.f, 0, 0, 1, M_PI/4);

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_g2l(v.f, b0, b2); // from space of b0 to space of b2
    EXPECT_VEC3_NEAR(v, -4.242640687119285, 0.0, -7.414213562373094, 1e-7);
}

TEST(NAME, g2l_3_disconnected_bones_rotation_and_without_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;  ik_vec3_set(b0->position.f, 1, 1, 1);  ik_quat_set_axis_angle(b0->rotation.f, 1, 0, 0, M_PI/4);
    b1->length = 3.0;  ik_vec3_set(b1->position.f, 2, 2, 2);  ik_quat_set_axis_angle(b1->rotation.f, 0, 1, 0, M_PI/4);
    b2->length = 4.0;  ik_vec3_set(b2->position.f, 3, 3, 3);  ik_quat_set_axis_angle(b2->rotation.f, 0, 0, 1, M_PI/4);

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_g2l(v.f, ik_bone_get_parent(b0), b2); // global space to space of b2
    EXPECT_VEC3_NEAR(v, -3.5104076400856536, -0.5606601717798212, -9.742640687119284, 1e-7);
}

TEST(NAME, l2g_2_bones_with_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;
    b1->length = 3.0;
    b2->length = 4.0;

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_l2g(v.f, b2, b0); // from space of b2 to space of b0
    EXPECT_VEC3_NEAR(v, 1, 2, 8, 1e-7);
}

TEST(NAME, l2g_3_bones_without_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;
    b1->length = 3.0;
    b2->length = 4.0;

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_l2g(v.f, b2, ik_bone_get_parent(b0)); // b2 to global space
    EXPECT_VEC3_NEAR(v, 1, 2, 8, 1e-7);
}

TEST(NAME, l2g_2_disconnected_bones_with_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;  ik_vec3_set(b0->position.f, 1, 1, 1);
    b1->length = 3.0;  ik_vec3_set(b1->position.f, 2, 2, 2);
    b2->length = 4.0;  ik_vec3_set(b2->position.f, 3, 3, 3);

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_l2g(v.f, b2, b0); // from space of b2 to space of b0
    EXPECT_VEC3_NEAR(v, 6, 7, 13, 1e-7);
}

TEST(NAME, l2g_3_disconnected_bones_without_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;  ik_vec3_set(b0->position.f, 1, 1, 1);
    b1->length = 3.0;  ik_vec3_set(b1->position.f, 2, 2, 2);
    b2->length = 4.0;  ik_vec3_set(b2->position.f, 3, 3, 3);

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_l2g(v.f, b2, ik_bone_get_parent(b0)); // b2 to global space
    EXPECT_VEC3_NEAR(v, 7, 8, 14, 1e-7);
}

TEST(NAME, l2g_2_disconnected_bones_with_rotation_and_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;  ik_vec3_set(b0->position.f, 1, 1, 1);  ik_quat_set_axis_angle(b0->rotation.f, 1, 0, 0, M_PI/4);
    b1->length = 3.0;  ik_vec3_set(b1->position.f, 2, 2, 2);  ik_quat_set_axis_angle(b1->rotation.f, 0, 1, 0, M_PI/4);
    b2->length = 4.0;  ik_vec3_set(b2->position.f, 3, 3, 3);  ik_quat_set_axis_angle(b2->rotation.f, 0, 0, 1, M_PI/4);

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_l2g(v.f, b2, b0); // from space of b2 to space of b0
    EXPECT_VEC3_NEAR(v, 9.98528137423857, 7.121320343559642, 8.742640687119286, 1e-7);
}

TEST(NAME, l2g_3_disconnected_bones_rotation_and_without_parent)
{
    ik::Ref<ik_bone> b0 = ik_bone_create(2);
    ik::Ref<ik_bone> b1 = ik_bone_create_child(b0, 2);
    ik::Ref<ik_bone> b2 = ik_bone_create_child(b1, 2);
    b0->length = 2.0;  ik_vec3_set(b0->position.f, 1, 1, 1);  ik_quat_set_axis_angle(b0->rotation.f, 1, 0, 0, M_PI/4);
    b1->length = 3.0;  ik_vec3_set(b1->position.f, 2, 2, 2);  ik_quat_set_axis_angle(b1->rotation.f, 0, 1, 0, M_PI/4);
    b2->length = 4.0;  ik_vec3_set(b2->position.f, 3, 3, 3);  ik_quat_set_axis_angle(b2->rotation.f, 0, 0, 1, M_PI/4);

    ik_vec3 v;
    ik_vec3_set(v.f, 1, 2, 3);

    ik_transform_bone_pos_l2g(v.f, b2, ik_bone_get_parent(b0)); // b2 to global space
    EXPECT_VEC3_NEAR(v, 10.98528137423857, -0.14644660940672782, 12.2175144212722, 1e-7);
}
