#include "ik/python/ik_type_Node.h"
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
        NULL
    };

    static int guid_counter = 0;

    self = (ik_Node*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    self->guid = -1;
    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|IO", kwds_str, &self->guid, &self->user))
        goto parse_args_failed;

    self->node = ik_node_create(ik_ptr(self));
    if (self->node == NULL)
        goto alloc_ik_node_failed;
    IK_INCREF(self->node);

    if (self->guid < 0)
        self->guid = guid_counter++;

    if (self->user == NULL)
    {
        Py_INCREF(Py_None);
        self->user = Py_None;
    }

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

/* ------------------------------------------------------------------------- */
static PyMethodDef Node_methods[] = {
    {"create_child", (PyCFunction)Node_create_child, METH_VARARGS | METH_KEYWORDS, NODE_CREATE_CHILD_DOC},
    {"link",         (PyCFunction)Node_link, METH_O, NODE_LINK_DOC},
    {"unlink",       (PyCFunction)Node_unlink, METH_NOARGS, NODE_UNLINK_DOC},
    {"pack",         (PyCFunction)Node_pack, METH_NOARGS, NODE_PACK_DOC},
    {"find",         (PyCFunction)Node_find, METH_O, NODE_FIND_DOC},
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
static PyGetSetDef Node_getset[] = {
    {"guid",        (getter)Node_getguid,        (setter)Node_setguid,        NODE_GUID_DOC,        NULL},
    {"user",        (getter)Node_getuser,        (setter)Node_setuser,        NODE_USER_DOC,        NULL},
    {"child_count", (getter)Node_getchild_count, (setter)Node_setchild_count, NODE_CHILD_COUNT_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_DOC,
"");
PyTypeObject ik_NodeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Node",
    .tp_basicsize = sizeof(ik_Node),
    .tp_dealloc = (destructor)Node_dealloc,
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
