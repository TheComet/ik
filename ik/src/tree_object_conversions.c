#include "ik/tree_object_conversions.h"
#include "ik/node.h"
#include "ik/bone.h"
#include "ik/log.h"
#include <stddef.h>

/* ------------------------------------------------------------------------- */
void
ik_node_to_bone(const struct ik_node* node_root, struct ik_bone* bone_root)
{

}

/* ------------------------------------------------------------------------- */
void
ik_bone_to_node(const struct ik_bone* bone_root, struct ik_node* node_root)
{

}

/* ------------------------------------------------------------------------- */
struct ik_bone*
ik_node_to_bone_inplace(struct ik_node* root)
{
    /*
     * Both trees occupy the same memory location. This requires that the tree
     * has been packed.
     */
    struct ik_bone* bone_root = (struct ik_bone*)root;

    /*
     * Try to help the user as much as possible in debug mode. Make sure that
     * the tree was packed and there is enough space in the refcounted memory
     * block for the conversion.
     */
#if !defined(NDEBUG)
    int capacity = IK_REFCOUNTED_OBJS(root);
    int total_nodes = ik_node_count(root);
    int leaf_nodes = ik_bone_leaf_count(root);

    if (capacity < total_nodes - leaf_nodes)
    {
        if (capacity == 1)
            ik_log_printf(IK_ERROR, "ik_node_to_bone_inplace(): You must pack the tree first with ik_node_pack()");
        else
            ik_log_printf(IK_ERROR, "ik_node_to_bone_inplace(): There is not enough memory to convert to bone representation. This can happen if you add new node after calling ik_node_pack(). Call ik_node_pack() again after modifying the tree before calling this function.");

        return NULL;
    }
#endif

    ik_node_to_bone(root, bone_root);
    return bone_root;
}

/* ------------------------------------------------------------------------- */
struct ik_node*
ik_bone_to_node_inplace(struct ik_bone* root)
{
    /*
     * Both trees occupy the same memory location. This requires that the tree
     * has been packed.
     */
    struct ik_node* node_root = (struct ik_node*)root;

    /*
     * There are always more objects in node representation than bone
     * representation, due to the fencepost problem. The only way to ensure
     * there is enough memory for the conversion is to check the size of the
     * refcounted array.
     *
     * Try to help the user as much as possible in debug mode.
     */
#if !defined(NDEBUG)
    int capacity = IK_REFCOUNTED_OBJS(root);
    int total_bones = ik_bone_count(root);
    int leaf_bones = ik_bone_leaf_count(root);

    if (capacity < total_bones + leaf_bones)
    {
        if (capacity == 1)
            ik_log_printf(IK_ERROR, "ik_bone_to_node_inplace(): You must pack the tree first with ik_bone_pack_for_inplace_conversion()");
        else
            ik_log_printf(IK_ERROR, "ik_bone_to_node_inplace(): There is not enough memory to convert to node representation. This can happen if you add new bones after calling ik_bone_pack_for_inplace_conversion(), or if you only use ik_bone_pack() instead of ik_bone_pack_for_inplace_conversion(). Make sure to use ik_bone_pack_for_inplace_conversion() after modifying the tree.");

        return NULL;
    }
#endif

    ik_bone_to_node(root, node_root);
    return node_root;
}
