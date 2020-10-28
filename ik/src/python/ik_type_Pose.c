#include "ik/python/ik_type_Bone.h"
#include "ik/python/ik_type_Pose.h"
#include "ik/python/ik_docstrings.h"
#include "ik/vec3.h"
#include "ik/bone.h"
#include "ik/pose.h"

/* ------------------------------------------------------------------------- */
static int
Pose_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    struct ik_pose* tmp;
    ik_Bone* root;
    ik_Pose* self = (ik_Pose*)myself;

    static char* kwds_str[] = {
        "root",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwds_str, &ik_BoneType, &root))
        return -1;

    tmp = self->pose;
    self->pose = ik_pose_alloc((struct ik_bone*)root->super.tree_object);
    if (self->pose == NULL)
    {
        self->pose = tmp;
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate pose buffer");
        return -1;
    }
    IK_INCREF(self->pose);
    IK_XDECREF(tmp);

    ik_pose_save(self->pose, (struct ik_bone*)root->super.tree_object);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Pose_apply(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Bone* root;
    ik_Pose* self = (ik_Pose*)myself;
    struct ik_bone* root_bone;
    union ik_vec3 root_pos;
    int skip_root_position = 1;

    static char* kwds_str[] = {
        "root",
        "skip_root_position",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|p", kwds_str,
        &ik_BoneType, &root,
        &skip_root_position))
    {
        return NULL;
    }

    root_bone = (struct ik_bone*)root->super.tree_object;

    root_pos = root_bone->position;
    ik_pose_apply(self->pose, root_bone);
    if (skip_root_position)
        root_bone->position = root_pos;

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Pose_methods[] = {
    {"apply", (PyCFunction)Pose_apply, METH_VARARGS | METH_KEYWORDS, IK_POSE_APPLY_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyTypeObject ik_PoseType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Pose",
    .tp_basicsize = sizeof(ik_Pose),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_POSE_DOC,
    .tp_methods = Pose_methods,
    .tp_init = Pose_init
};

/* ------------------------------------------------------------------------- */
int
init_ik_PoseType(void)
{
    if (PyType_Ready(&ik_PoseType) < 0)
        return -1;

    return 0;
}
