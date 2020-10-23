#pragma once

#include "ik/tree_object.h"
#include "ik/quat.h"
#include "ik/vec3.h"
#include "cstructures/vector.h"

C_BEGIN

struct ik_bone
{
    IK_TREE_OBJECT_HEAD

    union ik_quat rotation;
    union ik_vec3 position;
    ikreal length;
};

IK_PUBLIC_API struct ik_bone*
ik_bone_create(void);

IK_PUBLIC_API struct ik_bone*
ik_bone_create_child(struct ik_bone* parent);

static inline int
ik_bone_link(struct ik_bone* parent, struct ik_bone* child) {
    return ik_tree_object_link((struct ik_tree_object*)parent, (struct ik_tree_object*)child);
}

static inline int
ik_bone_can_link(const struct ik_bone* parent, const struct ik_bone* child) {
    return ik_tree_object_can_link((const struct ik_tree_object*)parent, (struct ik_tree_object*)child);
}

static inline void
ik_bone_unlink(struct ik_bone* bone) {
    ik_tree_object_unlink((struct ik_tree_object*)bone);
}

static inline void
ik_bone_unlink_all_children(struct ik_bone* bone) {
    ik_tree_object_unlink_all_children((struct ik_tree_object*)bone);
}

static inline struct ik_bone*
ik_bone_find(struct ik_bone* root, const void* user_data) {
    return (struct ik_bone*)ik_tree_object_find((struct ik_tree_object*)root, user_data);
}

#define ik_bone_child_count(bone) \
    (ik_tree_object_child_count(bone))

#define ik_bone_get_child(bone, idx) \
    ((struct ik_bone*)ik_tree_object_get_child(bone, idx))

#define ik_bone_get_parent(bone) \
    ((struct ik_bone*)(bone)->parent)

static inline int
ik_bone_count(const struct ik_bone* root)  {
    return ik_tree_object_count((struct ik_tree_object*)root);
}

static inline int
ik_bone_leaf_count(const struct ik_bone* root) {
    return ik_tree_object_leaf_count((struct ik_tree_object*)root);
}

static inline struct ik_bone*
ik_bone_duplicate_shallow(const struct ik_bone* root) {
    return (struct ik_bone*)ik_tree_object_duplicate_shallow(
        (const struct ik_tree_object*)root, sizeof *root, 0);
}

static inline struct ik_bone*
ik_bone_duplicate_full(const struct ik_bone* root) {
    return (struct ik_bone*)ik_tree_object_duplicate_full(
        (const struct ik_tree_object*)root, sizeof *root, 0);
}

IK_PUBLIC_API struct ik_node*
ik_bone_duplicate_shallow_for_node_transform(const struct ik_bone* root);

IK_PUBLIC_API struct ik_node*
ik_bone_duplicate_full_for_node_transform(const struct ik_bone* root);

#define X1(upper, lower, arg0)                                                \
        static inline struct ik_##lower*                                      \
        ik_bone_create_##lower(struct ik_bone* bone, arg0 arg) {              \
            return ik_tree_object_create_##lower((struct ik_tree_object*)bone, arg); \
        }
#define X(upper, lower)                                                       \
        static inline struct ik_##lower*                                      \
        ik_bone_create_##lower(struct ik_bone* bone) {                        \
            return ik_tree_object_create_##lower((struct ik_tree_object*)bone); \
        }
    IK_ATTACHMENT_LIST
#undef X
#undef X1

#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower)                                                       \
        static inline void                                                    \
        ik_bone_attach_##lower(struct ik_bone* bone, struct ik_##lower* lower) { \
            ik_tree_object_attach_##lower((struct ik_tree_object*)bone, lower); \
        }                                                                     \
                                                                              \
        static inline struct ik_##lower*                                      \
        ik_bone_detach_##lower(struct ik_bone* bone) {                        \
            return ik_tree_object_detach_##lower((struct ik_tree_object*)bone); \
        }
    IK_ATTACHMENT_LIST
#undef X
#undef X1

#define BONE_FOR_EACH_CHILD(bone, child) \
    VECTOR_FOR_EACH(&(bone)->children, struct ik_bone*, p##child) \
    struct ik_bone* child = *p##child; (void)child; {

#define BONE_END_EACH } VECTOR_END_EACH

C_END
