#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Algorithm.h"
#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_Pole.h"
#include "ik/node.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
Node_dealloc(ik_Node* self)
{
    IK_DECREF(self->node);
    Py_DECREF(self->algorithm);
    Py_DECREF(self->constraint);
    Py_DECREF(self->effector);
    Py_DECREF(self->pole);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Node* self;
    struct ik_node* node;

    (void)args; (void)kwds;

    node = ik_node_create(ik_ptr(0));
    if (node == NULL)
        goto alloc_node_failed;
    IK_INCREF(node);

    self = (ik_Node*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    /*
     * Store the python object in node's user data so we don't have to store
     * child nodes in a python list.
     */
    node->user.ptr = self;

    /* store node */
    self->node = node;

    /* Set all attachments to None */
    Py_INCREF(Py_None); self->algorithm = Py_None;
    Py_INCREF(Py_None); self->constraint = Py_None;
    Py_INCREF(Py_None); self->effector = Py_None;
    Py_INCREF(Py_None); self->pole = Py_None;

    return (PyObject*)self;

    alloc_self_failed : IK_DECREF(node);
    alloc_node_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Node_init(ik_Node* self, PyObject* args, PyObject* kwds)
{
    ik_Algorithm* algorithm = NULL;
    ik_Constraint* constraint = NULL;
    ik_Effector* effector = NULL;
    ik_Pole* pole = NULL;

    static char* kwds_str[] = {
        "algorithm",
        "constraint",
        "effector",
        "pole",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!O!O!O!", kwds_str,
            &ik_AlgorithmType, &algorithm,
            &ik_ConstraintType, &constraint,
            &ik_EffectorType, &effector,
            &ik_PoleType, &pole))
        return -1;

    #define X1(upper, lower, arg) X(upper, lower)
    #define X(upper, lower)                                                   \
        if (lower != NULL) {                                                  \
            PyObject* tmp;                                                    \
                                                                              \
            /* Attach to internal node */                                     \
            ik_Attachment* py_attachment = (ik_Attachment*)self->lower;       \
            ik_node_attach_##lower(self->node, (struct ik_##lower*)py_attachment->attachment); \
                                                                              \
            /* Set attachment on python object */                             \
            tmp = self->lower;                                                \
            Py_INCREF(py_attachment);                                         \
            self->lower = (PyObject*)py_attachment;                           \
            Py_DECREF(tmp);                                                   \
        }
        IK_ATTACHMENT_LIST
    #undef X
    #undef X1

    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_CREATE_CHILD_DOC, "");
static PyObject*
Node_create_child(ik_Node* self, PyObject* args, PyObject* kwds)
{
    (void)args;
    ik_Node* child = (ik_Node*)PyObject_Call((PyObject*)&ik_NodeType, args, kwds);
    if (child == NULL)
        return NULL;

    if (ik_node_link(self->node, child->node) != 0)
    {
        Py_DECREF(child);
        PyErr_SetString(PyExc_RuntimeError, "Failed to link new node (out of memory?) Check log for more info.");
        return NULL;
    }

    return (PyObject*)child;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_LINK_DOC, "");
static PyObject*
Node_link(ik_Node* self, PyObject* node)
{
    ik_Node* other;

    if (!ik_Node_CheckExact(node))
    {
        PyErr_SetString(PyExc_TypeError, "Argument must be of type ik.Node");
        return NULL;
    }

    other = (ik_Node*)node;
    if (ik_node_link(self->node, other->node) != 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to link node (out of memory?) Check log for more info.");
        return NULL;
    }

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_UNLINK_DOC, "");
static PyObject*
Node_unlink(ik_Node* self, PyObject* args)
{
    (void)args;
    ik_node_unlink(self->node);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_PACK_DOC, "");
static PyObject*
Node_pack(ik_Node* self, PyObject* arg)
{
    (void)arg;
    ik_node_pack(self->node);
    Py_RETURN_NONE;
}


/* ------------------------------------------------------------------------- */
static PyMethodDef Node_methods[] = {
    {"create_child",      (PyCFunction)Node_create_child,      METH_VARARGS | METH_KEYWORDS, NODE_CREATE_CHILD_DOC},
    {"link",              (PyCFunction)Node_link,              METH_O,                       NODE_LINK_DOC},
    {"unlink",            (PyCFunction)Node_unlink,            METH_NOARGS,                  NODE_UNLINK_DOC},
    {"pack",              (PyCFunction)Node_pack,              METH_NOARGS,                  NODE_PACK_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_CHILD_COUNT_DOC,"");
static PyObject*
Node_getchild_count(ik_Node* self, void* closure)
{
    (void)closure;
    return PyLong_FromLong(ik_node_child_count(self->node));
}
static int
Node_setchild_count(ik_Node* self, PyObject* value, void* closure)
{
    (void)self; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "Child count is read-only");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_ALGORITHM_DOC,"");
PyDoc_STRVAR(NODE_CONSTRAINT_DOC,"");
PyDoc_STRVAR(NODE_EFFECTOR_DOC,"");
PyDoc_STRVAR(NODE_POLE_DOC,"");
#define X1(upper, lower, arg) X(upper, lower)
#define X(upper, lower)                                                       \
    static PyObject*                                                          \
    Node_get##lower(ik_Node* self, void* closure)                             \
    {                                                                         \
        (void)closure;                                                        \
        return Py_INCREF(self->lower), self->lower;                           \
    }                                                                         \
                                                                              \
    static int                                                                \
    Node_set##lower(ik_Node* self, PyObject* value, void* closure)            \
    {                                                                         \
        (void)closure;                                                        \
                                                                              \
        if (ik_##lower##_CheckExact(value))                                   \
        {                                                                     \
            PyObject* tmp;                                                    \
            ik_Attachment* py_attachment = (ik_Attachment*)value;             \
            ik_node_attach_##lower(self->node, (struct ik_##lower*)py_attachment->attachment); \
                                                                              \
            tmp = self->lower;                                                \
            Py_INCREF(value);                                                 \
            self->lower = value;                                              \
            Py_DECREF(tmp);                                                   \
                                                                              \
            return 0;                                                         \
        }                                                                     \
                                                                              \
        if (value == Py_None)                                                 \
        {                                                                     \
            PyObject* tmp;                                                    \
            ik_node_detach_##lower(self->node);                               \
                                                                              \
            tmp = self->lower;                                                \
            Py_INCREF(Py_None);                                               \
            self->lower = Py_None;                                            \
            Py_DECREF(tmp);                                                   \
                                                                              \
            return 0;                                                         \
        }                                                                     \
                                                                              \
        PyErr_SetString(PyExc_TypeError, "Must assign an instance of type " #lower " or None"); \
        return -1; \
    }
    IK_ATTACHMENT_LIST
#undef X
#undef X1

/* ------------------------------------------------------------------------- */
static PyGetSetDef Node_getset[] = {
    {"child_count", (getter)Node_getchild_count, (setter)Node_setchild_count, NODE_CHILD_COUNT_DOC, NULL},
#define X1(upper, lower, arg) X(upper, lower)
#define X(upper, lower) \
    {#lower,        (getter)Node_get##lower,     (setter)Node_set##lower,     NODE_##upper##_DOC,   NULL},
    IK_ATTACHMENT_LIST
#undef X
#undef X1
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Node_repr(ik_Node* self)
{
    return PyUnicode_FromFormat("ik.Node()");
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_str(ik_Node* self)
{
    return Node_repr(self);
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_DOC,
"");
PyTypeObject ik_NodeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Node",
    .tp_basicsize = sizeof(ik_Node),
    .tp_dealloc = (destructor)Node_dealloc,
    .tp_repr = (reprfunc)Node_repr,
    .tp_str = (reprfunc)Node_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = NODE_DOC,
    .tp_methods = Node_methods,
    .tp_getset = Node_getset,
    .tp_new = Node_new,
    .tp_init = (initproc)Node_init
};

/* ------------------------------------------------------------------------- */
int
init_ik_NodeType(void)
{
    if (PyType_Ready(&ik_NodeType) < 0)
        return -1;
    return 0;
}
