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
    Py_XDECREF(self->user);
    IK_DECREF(self->node);
    Py_TYPE(self)->tp_free((PyObject*)self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Node* self;

    static char* kwds_str[] = {
        "guid",
        "user",
        "algorithm",
        "constraint",
        "effector",
        "pole",
        NULL
    };

    static int guid_counter = 0;

    self = (ik_Node*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    self->guid = -1;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|IOO!O!O!O!", kwds_str,
            &self->guid,
            &self->user,
            &ik_AlgorithmType, &self->algorithm,
            &ik_ConstraintType, &self->constraint,
            &ik_EffectorType, &self->effector,
            &ik_PoleType, &self->pole))
        goto parse_args_failed;

    self->node = ik_node_create(ik_ptr(self));
    if (self->node == NULL)
        goto alloc_ik_node_failed;
    IK_INCREF(self->node);

    if (self->guid < 0)
        self->guid = guid_counter++;

    if (self->user == NULL)
        self->user = Py_None;

#define X1(upper, lower, arg) X(upper, lower)
#define X(upper, lower) \
    if (self->lower) \
        ik_node_attach_##lower(self->node, (struct ik_##lower*)(((ik_Attachment*)self->lower)->attachment)); \
    else \
        self->lower = Py_None; \
    Py_INCREF(self->lower);
    IK_ATTACHMENT_LIST
#undef X
#undef X1

    Py_INCREF(self->user);

    return (PyObject*)self;

    alloc_ik_node_failed :
    parse_args_failed    : type->tp_free((PyObject*)self);
    alloc_self_failed    : return NULL;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_CREATE_CHILD_DOC, "");
static PyObject*
Node_create_child(ik_Node* self, PyObject* args, PyObject* kwds)
{
    (void)args;
    ik_Node* child = (ik_Node*)Node_new(&ik_NodeType, args, kwds);
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
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_FIND_DOC, "");
static PyObject*
Node_find_recursive(ik_Node* self, int guid)
{
    if (self->guid == guid)
        return Py_INCREF(self), (PyObject*)self;

    NODE_FOR_EACH(self->node, node_data, child)
        PyObject* result;
        ik_Node* py_child = child->user.ptr;
        assert(py_child != NULL);
        if ((result = Node_find_recursive(py_child, guid)))
            return result;
    NODE_END_EACH

    Py_RETURN_NONE;
}
static PyObject*
Node_find(ik_Node* self, PyObject* guid)
{
    if (!PyLong_Check(guid))
    {
        PyErr_SetString(PyExc_TypeError, "Argument must be an integer (guid)");
        return NULL;
    }

    return (PyObject*)Node_find_recursive(self, PyLong_AS_LONG(guid));
}

/* ------------------------------------------------------------------------- */
static PyMethodDef Node_methods[] = {
    {"create_child",      (PyCFunction)Node_create_child,      METH_VARARGS | METH_KEYWORDS, NODE_CREATE_CHILD_DOC},
    {"link",              (PyCFunction)Node_link,              METH_O,                       NODE_LINK_DOC},
    {"unlink",            (PyCFunction)Node_unlink,            METH_NOARGS,                  NODE_UNLINK_DOC},
    {"pack",              (PyCFunction)Node_pack,              METH_NOARGS,                  NODE_PACK_DOC},
    {"find",              (PyCFunction)Node_find,              METH_O,                       NODE_FIND_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_GUID_DOC,"");
static PyObject*
Node_getguid(ik_Node* self, void* closure)
{
    (void)closure;
    return PyLong_FromLong(self->guid);
}
static int
Node_setguid(ik_Node* self, PyObject* value, void* closure)
{
    (void)self; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "GUID can't be changed after node construction");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_USER_DOC,"");
static PyObject*
Node_getuser(ik_Node* self, void* closure)
{
    (void)closure;
    return Py_INCREF(self->user), self->user;
}
static int
Node_setuser(ik_Node* self, PyObject* value, void* closure)
{
    (void)self; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "User object can't be changed after node construction");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_CHILD_COUNT_DOC,"");
static PyObject*
Node_getchild_count(ik_Node* self, void* closure)
{
    (void)closure;
    return Py_INCREF(self->user), self->user;
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
            ik_Attachment* lower = (ik_Attachment*)value;                     \
            ik_node_attach_##lower(self->node, (struct ik_##lower*)lower->attachment); \
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
    {"guid",        (getter)Node_getguid,        (setter)Node_setguid,        NODE_GUID_DOC,        NULL},
    {"user",        (getter)Node_getuser,        (setter)Node_setuser,        NODE_USER_DOC,        NULL},
    {"child_count", (getter)Node_getchild_count, (setter)Node_setchild_count, NODE_CHILD_COUNT_DOC, NULL},
    {"algorithm",   (getter)Node_getalgorithm,   (setter)Node_setalgorithm,   NODE_ALGORITHM_DOC,   NULL},
    {"constraint",  (getter)Node_getconstraint,  (setter)Node_setconstraint,  NODE_CONSTRAINT_DOC,  NULL},
    {"effector",    (getter)Node_geteffector,    (setter)Node_seteffector,    NODE_EFFECTOR_DOC,    NULL},
    {"pole",        (getter)Node_getpole,        (setter)Node_setpole,        NODE_POLE_DOC,        NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Node_repr(ik_Node* self)
{
    return PyUnicode_FromFormat("Node(guid=%d, user=%R)", self->guid, self->user);
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
    .tp_new = Node_new
};

/* ------------------------------------------------------------------------- */
int
init_ik_NodeType(void)
{
    if (PyType_Ready(&ik_NodeType) < 0)
        return -1;
    return 0;
}
