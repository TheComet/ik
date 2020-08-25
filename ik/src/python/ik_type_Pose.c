#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Pose.h"
#include "ik/python/ik_docstrings.h"
#include "ik/vec3.h"
#include "ik/node.h"
#include "ik/pose.h"

/* ------------------------------------------------------------------------- */
static void
Pose_dealloc(PyObject* myself)
{
    ik_Pose* self = (ik_Pose*)myself;
    IK_XDECREF(self->pose);
    ik_PoseType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Pose_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Pose* self = (ik_Pose*)ik_PoseType.tp_base->tp_new(type, args, kwds);
    self->pose = NULL;
    return (PyObject*)self;
}

/* ------------------------------------------------------------------------- */
static int
Pose_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    struct ik_pose* tmp;
    ik_Node* root;
    ik_Pose* self = (ik_Pose*)myself;

    static char* kwds_str[] = {
        "root",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!", kwds_str, &ik_NodeType, &root))
        return -1;

    tmp = self->pose;
    self->pose = ik_pose_alloc(root->node);
    if (self->pose == NULL)
    {
        self->pose = tmp;
        PyErr_SetString(PyExc_MemoryError, "Failed to allocate pose buffer");
        return -1;
    }
    IK_INCREF(self->pose);
    IK_XDECREF(tmp);

    ik_pose_save(self->pose, root->node);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Pose_apply(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Node* root;
    ik_Pose* self = (ik_Pose*)myself;
    union ik_vec3 root_pos;
    int skip_root_position = 1;

    static char* kwds_str[] = {
        "root",
        "skip_root_position",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "O!|p", kwds_str,
        &ik_NodeType, &root,
        &skip_root_position))
    {
        return NULL;
    }

    root_pos = root->node->position;
    ik_pose_apply(self->pose, root->node);
    if (skip_root_position)
        root->node->position = root_pos;

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
    .tp_dealloc = Pose_dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_POSE_DOC,
    .tp_methods = Pose_methods,
    .tp_new = Pose_new,
    .tp_init = Pose_init
};

/* ------------------------------------------------------------------------- */
int
init_ik_PoseType(void)
{
    ik_PoseType.tp_base = &ik_ModuleRefType;
    if (PyType_Ready(&ik_PoseType) < 0)
        return -1;

    return 0;
}
