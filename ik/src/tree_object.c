#include "ik/tree_object.h"
#include "ik/log.h"
#include <stddef.h>
#include <string.h>

/* ------------------------------------------------------------------------- */
static void
ik_tree_object_deinit_shallow(struct ik_tree_object* tree_object)
{
    assert(tree_object);

    /* unref attachments */
#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower) ik_tree_object_detach_##lower(tree_object);
    IK_ATTACHMENT_LIST
#undef X
#undef X1

    /* deinit */
    vector_deinit(&tree_object->children);
}

/* ------------------------------------------------------------------------- */
static void
tree_object_deinit(struct ik_tree_object* tree_object)
{
    assert(tree_object);

    /* Unref children */
    TREE_OBJECT_FOR_EACH_CHILD(tree_object, child)
        IK_DECREF(child);
    TREE_OBJECT_END_EACH

    ik_tree_object_deinit_shallow(tree_object);
}

/* ------------------------------------------------------------------------- */
static void
proxy_refcount_deinit(struct ik_tree_object* obj)
{
    struct ik_refcounted* proxy_ref = ik_refcount_to_first_obj_address(obj->refcount);
    tree_object_deinit(obj);
    IK_DECREF(proxy_ref);
}

/* ------------------------------------------------------------------------- */
static void
ik_tree_object_destroy(struct ik_tree_object* tree_object)
{
    tree_object_deinit(tree_object);
    ik_refcounted_obj_free((struct ik_refcounted*)tree_object);
}

/* ------------------------------------------------------------------------- */
static void
ik_tree_object_init(struct ik_tree_object* tree_object)
{
    tree_object->user_data = NULL;
    tree_object->algorithm = NULL;
    tree_object->constraint = NULL;
    tree_object->effector = NULL;
    tree_object->pole = NULL;

    tree_object->rotation_weight = 1.0;
    tree_object->mass = 1.0;

    tree_object->parent = NULL;
    vector_init(&tree_object->children, sizeof(struct ik_tree_object*));
}

/* ------------------------------------------------------------------------- */
struct ik_tree_object*
ik_tree_object_create(uintptr_t derived_size)
{
    struct ik_tree_object* tree_object = (struct ik_tree_object*)
        ik_refcounted_alloc(derived_size, (ik_deinit_func)tree_object_deinit);
    if (tree_object == NULL)
        return NULL;

    ik_tree_object_init(tree_object);
    return tree_object;
}

/* ------------------------------------------------------------------------- */
struct ik_tree_object*
ik_tree_object_create_child(struct ik_tree_object* parent, uintptr_t derived_size)
{
    struct ik_tree_object* child;
    if ((child = ik_tree_object_create(derived_size)) == NULL)
        goto create_child_failed;
    if (ik_tree_object_link(parent, child) != 0)
        goto add_child_failed;

    return child;

    add_child_failed    : ik_tree_object_destroy(child);
    create_child_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
int
ik_tree_object_link(struct ik_tree_object* tree_object, struct ik_tree_object* child)
{
    assert(tree_object);
    assert(child);

    if (vector_push(&tree_object->children, &child) != 0)
    {
        ik_log_out_of_memory("ik_tree_object_link()");
        return -1;
    }

    /* May already be part of a tree */
    IK_INCREF(child);
    ik_tree_object_unlink(child);
    child->parent = tree_object;

    return 0;
}

/* ------------------------------------------------------------------------- */
int
ik_tree_object_can_link(const struct ik_tree_object* parent, const struct ik_tree_object* child)
{
    do {
        if (parent == child)
            return 0;
        parent = parent->parent;
    } while(parent);

    return 1;
}

/* ------------------------------------------------------------------------- */
void
ik_tree_object_unlink(struct ik_tree_object* tree_object)
{
    cs_vec_idx idx;
    assert(tree_object);

    if (tree_object->parent == NULL)
        return;

    idx = vector_find_element(&tree_object->parent->children, &tree_object);
    if (idx != vector_count(&tree_object->parent->children))
        vector_erase_index(&tree_object->parent->children, idx);

    tree_object->parent = NULL;
    IK_DECREF(tree_object);
}

/* ------------------------------------------------------------------------- */
void
ik_tree_object_unlink_all_children(struct ik_tree_object* tree_object)
{
    TREE_OBJECT_FOR_EACH_CHILD(tree_object, child)
        child->parent = NULL;
        IK_DECREF(child);
    TREE_OBJECT_END_EACH
    vector_clear(&tree_object->children);
}

/* ------------------------------------------------------------------------- */
int
ik_tree_object_count(const struct ik_tree_object* tree_object)
{
    int count = 1;
    TREE_OBJECT_FOR_EACH_CHILD(tree_object, child)
        count += ik_tree_object_count(child);
    TREE_OBJECT_END_EACH
    return count;
}

/* ------------------------------------------------------------------------- */
int
ik_tree_object_leaf_count(const struct ik_tree_object* node)
{
    int count = ik_tree_object_child_count(node) > 0 ? 0 : 1;
    TREE_OBJECT_FOR_EACH_CHILD(node, child)
        count += ik_tree_object_leaf_count(child);
    TREE_OBJECT_END_EACH
    return count;
}

/* ------------------------------------------------------------------------- */
static int
copy_tree_object_recurse(const struct ik_tree_object* src,
                         struct ik_refcount* proxy_ref_refcount,
                         void** obj_buf, uintptr_t obj_size,
                         struct ik_tree_object* parent,
                         int add_extra_leaf_objects)
{
    struct ik_refcounted* proxy_ref;
    struct ik_tree_object* obj;

    /* obj_buf is a pointer to a contiguous array of uninitialized tree objects.
     * Get a slot and initialize it */
    obj = *obj_buf;
    *obj_buf = (void**)((uintptr_t)(*obj_buf) + obj_size);
    ik_tree_object_init(obj);

    /* Set up proxy ref */
    proxy_ref = ik_refcounted_alloc(sizeof(*proxy_ref), (ik_deinit_func)proxy_refcount_deinit);
    if (proxy_ref == NULL)
        goto alloc_obj_refcount_failed;
    obj->refcount = ik_refcounted_obj_base_address(proxy_ref);
    proxy_ref->refcount = proxy_ref_refcount;
    IK_INCREF(proxy_ref);

    /* Init relations */
    if (parent)
    {
        if (ik_tree_object_link(parent, obj) != 0)
            goto link_to_parent_failed;
    }

    /* Copy data from source object. Can be NULL in the case of adding extra
     * leaf nodes */
    if (src != NULL)
    {
        /* Copy trivial fields */
        obj->user_data = src->user_data;
        obj->rotation_weight = src->rotation_weight;
        obj->mass = src->mass;

        /* Copy remaining data from derived type (ik_bone and ik_node have extra
        * fields that we don't directly have access to, so use memcpy) */
        memcpy((void*)((uintptr_t)obj + sizeof(struct ik_tree_object)),
               (const void*)((uintptr_t)src + sizeof(struct ik_tree_object)),
               obj_size - sizeof(struct ik_tree_object));

        TREE_OBJECT_FOR_EACH_CHILD(src, child)
            if (copy_tree_object_recurse(child, proxy_ref_refcount, obj_buf, obj_size, obj, add_extra_leaf_objects) != 0)
                goto copy_children_failed;
        TREE_OBJECT_END_EACH

        if (add_extra_leaf_objects && ik_tree_object_child_count(src) == 0)
        {
            if (copy_tree_object_recurse(NULL, proxy_ref_refcount, obj_buf, obj_size, obj, add_extra_leaf_objects) != 0)
                goto copy_children_failed;
        }
    }

    return 0;

    copy_children_failed      :
    link_to_parent_failed     : ik_refcounted_obj_free(proxy_ref);
    alloc_obj_refcount_failed : ik_tree_object_deinit_shallow(obj);
    return -1;
}

/* ------------------------------------------------------------------------- */
struct ik_tree_object*
ik_tree_object_duplicate_no_attachments(const struct ik_tree_object* root, uintptr_t obj_size, int add_extra_leaf_objects)
{
    int count;
    void* obj_buf;
    struct ik_tree_object* new_root;
    struct ik_refcount* proxy_ref_refcount;

    /*
     *         __________
     *        | refcount |<-----------------.
     *         ----------                   |
     *     .--| ref*     |                  |
     *     |   ----------                   |
     *     |                                |
     *     |   __________                   |
     *     |  | refcount |<--.              |
     *     |   ----------    |              |
     *     |--| ref*     |   |              |
     *     |   ----------    |              |
     *     |             .---               |
     *    _v________ ____|_ _____ _____ ____|_ _____ _____ _____
     *   | refcount | ref* | obj | der | ref* | obj | der | ...
     *     ^          ^
     *     |          |
     *     |     obj_buf points here
     *     |
     *  proxy_ref_refcount points here
     *
     */

    count = ik_tree_object_count(root);
    if (add_extra_leaf_objects)
        count += ik_tree_object_leaf_count(root);

    obj_buf = ik_refcounted_alloc(count * obj_size, NULL);
    if (obj_buf == NULL)
        goto alloc_new_tree_failed;

    proxy_ref_refcount = ik_refcounted_obj_base_address(obj_buf);

    new_root = obj_buf;

    if (copy_tree_object_recurse(root, proxy_ref_refcount, &obj_buf, obj_size, NULL, add_extra_leaf_objects) != 0)
        goto copy_tree_object_failed;

    return new_root;

    copy_tree_object_failed : ik_refcounted_obj_free((struct ik_refcounted*)new_root);
    alloc_new_tree_failed   : return NULL;
}

/* ------------------------------------------------------------------------- */
struct ik_tree_object*
ik_tree_object_duplicate_shallow(const struct ik_tree_object* root, uintptr_t obj_size, int add_extra_leaf_objects)
{
    struct ik_tree_object* new_root = ik_tree_object_duplicate_no_attachments(root, obj_size, add_extra_leaf_objects);
    if (new_root == NULL)
        return NULL;

    ik_attachment_reference_from_tree(new_root, root);

    return new_root;
}

/* ------------------------------------------------------------------------- */
struct ik_tree_object*
ik_tree_object_duplicate_full(const struct ik_tree_object* root, uintptr_t obj_size, int add_extra_leaf_objects)
{
    struct ik_tree_object* new_root = ik_tree_object_duplicate_no_attachments(root, obj_size, add_extra_leaf_objects);
    if (new_root == NULL)
        goto duplicate_tree_failed;

    if (ik_attachment_duplicate_from_tree(new_root, root) != 0)
        goto duplicate_attachments_failed;

    return new_root;

    duplicate_attachments_failed : ik_tree_object_destroy(new_root);
    duplicate_tree_failed        : return NULL;
}

/* ------------------------------------------------------------------------- */
struct ik_tree_object*
ik_tree_object_find(struct ik_tree_object* tree_object, const void* user_data)
{
    /* might be searching for this tree_object? */
    if (tree_object->user_data == user_data)
        return (struct ik_tree_object*)tree_object;

    TREE_OBJECT_FOR_EACH_CHILD(tree_object, child)
        struct ik_tree_object* found_child;
        if ((found_child = ik_tree_object_find(child, user_data)) != NULL)
            return found_child;
    TREE_OBJECT_END_EACH

    return NULL;
}
#define X1(upper, lower, argtype)                                             \
    struct ik_##lower*                                                        \
    ik_tree_object_create_##lower(struct ik_tree_object* tree_object, argtype arg) { \
        struct ik_##lower* lower = ik_##lower##_create(arg);                  \
        if (lower == NULL)                                                    \
            return NULL;                                                      \
                                                                              \
        ik_tree_object_attach_##lower(tree_object, lower);                    \
        return lower;                                                         \
    }
#define X(upper, lower)                                                       \
    struct ik_##lower*                                                        \
    ik_tree_object_create_##lower(struct ik_tree_object* tree_object) {       \
        struct ik_##lower* lower = ik_##lower##_create();                     \
        if (lower == NULL)                                                    \
            return NULL;                                                      \
                                                                              \
        ik_tree_object_attach_##lower(tree_object, lower);                    \
        return lower;                                                         \
    }
IK_ATTACHMENT_LIST
#undef X
#undef X1

/* ------------------------------------------------------------------------- */
#define X1(upper, lower, arg0) X(upper, lower)
#define X(upper, lower)                                                       \
    void                                                                      \
    ik_tree_object_attach_##lower(struct ik_tree_object* tree_object, struct ik_##lower* lower) { \
        IK_INCREF(lower);                                                     \
        ik_tree_object_detach_##lower(tree_object);                           \
        tree_object->lower = lower;                                           \
    }                                                                         \
                                                                              \
    struct ik_##lower*                                                        \
    ik_tree_object_detach_##lower(struct ik_tree_object* tree_object) {       \
        struct ik_##lower* lower = tree_object->lower;                        \
        tree_object->lower = NULL;                                            \
        if (lower) {                                                          \
            IK_DECREF(lower);                                                 \
        }                                                                     \
        return lower;                                                         \
    }

IK_ATTACHMENT_LIST
#undef X
#undef X1
