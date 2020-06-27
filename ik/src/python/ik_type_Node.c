#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Algorithm.h"
#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_Pole.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/node.h"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
NodeChildrenView_dealloc(ik_NodeChildrenView* self)
{
    IK_DECREF(self->node);
    Py_TYPE(self)->tp_free(self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
NodeChildrenView_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_NodeChildrenView* self;
    PyObject* node_capsule;

    (void)kwds;

    if (!PyArg_ParseTuple(args, "O!", &PyCapsule_Type, &node_capsule))
        return NULL;

    self = (ik_NodeChildrenView*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    self->node = PyCapsule_GetPointer(node_capsule, NULL);
    IK_INCREF(self->node);

    return (PyObject*)self;

    alloc_self_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
NodeChildrenView_repr(PyObject* myself)
{
    ik_NodeChildrenView* self = (ik_NodeChildrenView*)myself;
    return PyUnicode_FromFormat("ik.NodeChildrenView()");
}

/* ------------------------------------------------------------------------- */
static PyObject*
NodeChildrenView_str(PyObject* myself)
{
    return NodeChildrenView_repr(myself);
}

/* ------------------------------------------------------------------------- */
static Py_ssize_t
NodeChildrenView_length(PyObject* myself)
{
    ik_NodeChildrenView* self = (ik_NodeChildrenView*)myself;

    return ik_node_child_count(self->node);
}

/* ------------------------------------------------------------------------- */
static PyObject*
NodeChildrenView_item(PyObject* myself, Py_ssize_t index)
{
    ik_Node* node;
    ik_NodeChildrenView* self = (ik_NodeChildrenView*)myself;

    if (index < 0 || index >= ik_node_child_count(self->node))
    {
        PyErr_SetString(PyExc_IndexError, "Node child index out of range");
        return NULL;
    }

    node = ik_node_get_child(self->node, index)->user.ptr;
    return Py_INCREF(node), (PyObject*)node;
}

/* ------------------------------------------------------------------------- */
static PyObject*
NodeChildrenView_subscript(PyObject* self, PyObject* item)
{
    if (PyIndex_Check(item))
    {
        Py_ssize_t idx = PyNumber_AsSsize_t(item, PyExc_IndexError);
        if (idx == -1 && PyErr_Occurred())
            return NULL;
        return NodeChildrenView_item(self, idx);
    }
    else if (PySlice_Check(item))
    {
        PyErr_SetString(PyExc_TypeError, "Node children can't be sliced (they're not ordered)");
        return NULL;
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "Node child index must be an integer");
        return NULL;
    }
}

/* ------------------------------------------------------------------------- */
static PyMappingMethods NodeChildrenView_as_mapping = {
    .mp_length = NodeChildrenView_length,
    .mp_subscript = NodeChildrenView_subscript
};

/* ------------------------------------------------------------------------- */
static PySequenceMethods NodeChildrenView_as_sequence = {
    .sq_length = NodeChildrenView_length,
    .sq_item = NodeChildrenView_item
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_CHILDREN_VIEW_DOC,
"");
PyTypeObject ik_NodeChildrenViewType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.NodeChildrenView",
    .tp_basicsize = sizeof(ik_NodeChildrenView),
    .tp_dealloc = (destructor)NodeChildrenView_dealloc,
    .tp_repr = NodeChildrenView_repr,
    .tp_str = NodeChildrenView_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = NODE_CHILDREN_VIEW_DOC,
    .tp_new = NodeChildrenView_new,
    .tp_as_mapping = &NodeChildrenView_as_mapping,
    .tp_as_sequence = &NodeChildrenView_as_sequence
};

/* ------------------------------------------------------------------------- */
static void
Node_dealloc(ik_Node* self)
{
    IK_DECREF(self->node);
    Py_DECREF(self->algorithm);
    Py_DECREF(self->constraint);
    Py_DECREF(self->effector);
    Py_DECREF(self->pole);
    Py_DECREF(self->children);
    Py_TYPE(self)->tp_free(self);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Node* self;
    ik_NodeChildrenView* children_view;
    struct ik_node* node;
    PyObject* node_capsule;
    PyObject* constructor_args;

    (void)args; (void)kwds;

    /* Allocate the internal node */
    node = ik_node_create(ik_ptr(0));
    if (node == NULL)
        goto alloc_node_failed;
    IK_INCREF(node);

    /* Add node to capsule so we can construct a NodeChildrenView object */
    node_capsule = PyCapsule_New(node, NULL, NULL);
    if (node_capsule == NULL)
        goto alloc_children_view_failed;

    /* Add capsule to arglist tuple */
    constructor_args = PyTuple_New(1);
    if (constructor_args == NULL)
    {
        Py_DECREF(node_capsule);
        goto alloc_children_view_failed;
    }
    PyTuple_SET_ITEM(constructor_args, 0, node_capsule); /* steals ref */

    /* create children view object */
    children_view = (ik_NodeChildrenView*)PyObject_CallObject((PyObject*)&ik_NodeChildrenViewType, constructor_args);
    Py_DECREF(constructor_args); /* destroys arglist and capsule */
    if (children_view == NULL)
        goto alloc_children_view_failed;

    /* Finally, alloc self */
    self = (ik_Node*)type->tp_alloc(type, 0);
    if (self == NULL)
        goto alloc_self_failed;

    /*
     * Store the python object in node's user data so we don't have to store
     * child nodes in a python list.
     */
    node->user.ptr = self;

    /* store other objects we successfully allocated */
    self->node = node;
    self->children = children_view;

    /* Set all attachments to None */
    Py_INCREF(Py_None); self->algorithm = Py_None;
    Py_INCREF(Py_None); self->constraint = Py_None;
    Py_INCREF(Py_None); self->effector = Py_None;
    Py_INCREF(Py_None); self->pole = Py_None;

    return (PyObject*)self;

    alloc_self_failed          : Py_DECREF(children_view);
    alloc_children_view_failed : IK_DECREF(node);
    alloc_node_failed          : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Node_init(ik_Node* self, PyObject* args, PyObject* kwds)
{
    ik_Algorithm* algorithm = NULL;
    ik_Constraint* constraint = NULL;
    ik_Effector* effector = NULL;
    ik_Pole* pole = NULL;
    ik_Vec3* position = NULL;
    ik_Quat* rotation = NULL;

    static char* kwds_str[] = {
        "algorithm",
        "constraint",
        "effector",
        "pole",
        "position",
        "rotation",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!O!O!O!O!O!", kwds_str,
            &ik_AlgorithmType, &algorithm,
            &ik_ConstraintType, &constraint,
            &ik_EffectorType, &effector,
            &ik_PoleType, &pole,
            &ik_Vec3Type, &position,
            &ik_QuatType, &rotation))
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

    if (position != NULL)
        ik_vec3_copy(self->node->position.f, position->vec.f);
    if (rotation != NULL)
        ik_quat_copy(self->node->rotation.f, rotation->quat.f);

    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_CREATE_CHILD_DOC, "");
static PyObject*
Node_create_child(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Node* child;
    ik_Node* self = (ik_Node*)myself;

    (void)args;

    child = (ik_Node*)PyObject_Call((PyObject*)&ik_NodeType, args, kwds);
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
Node_link(PyObject* myself, PyObject* node)
{
    ik_Node* other;
    ik_Node* self = (ik_Node*)myself;

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
Node_unlink(PyObject* myself, PyObject* args)
{
    ik_Node* self = (ik_Node*)myself;
    (void)args;

    ik_node_unlink(self->node);
    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_PACK_DOC, "");
static PyObject*
Node_pack(PyObject* myself, PyObject* arg)
{
    ik_Node* self = (ik_Node*)myself;
    (void)arg;
    ik_node_pack(self->node);
    Py_RETURN_NONE;
}


/* ------------------------------------------------------------------------- */
static PyMethodDef Node_methods[] = {
    {"create_child", (PyCFunction)Node_create_child, METH_VARARGS | METH_KEYWORDS, NODE_CREATE_CHILD_DOC},
    {"link",         Node_link,                      METH_O,                       NODE_LINK_DOC},
    {"unlink",       Node_unlink,                    METH_NOARGS,                  NODE_UNLINK_DOC},
    {"pack",         Node_pack,                      METH_NOARGS,                  NODE_PACK_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_COUNT_DOC,"");
static PyObject*
Node_getcount(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return PyLong_FromLong(ik_node_count(self->node));
}
static int
Node_setcount(PyObject* myself, PyObject* value, void* closure)
{
    (void)myself; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "Count is read-only");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_CHILD_COUNT_DOC,"");
static PyObject*
Node_getchild_count(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return PyLong_FromLong(ik_node_child_count(self->node));
}
static int
Node_setchild_count(PyObject* myself, PyObject* value, void* closure)
{
    (void)myself; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "Child count is read-only");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_CHILDREN_DOC,"");
static PyObject*
Node_getchildren(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return Py_INCREF(self->children), (PyObject*)self->children;
}
static int
Node_setchildren(PyObject* myself, PyObject* value, void* closure)
{
    (void)myself; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "children property is read-only");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_PARENT_DOC,"");
static PyObject*
Node_getparent(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;

    if (self->node->parent != NULL)
    {
        ik_Node* parent = self->node->parent->user.ptr;
        return Py_INCREF(parent), (PyObject*)parent;
    }
    else
    {
        Py_RETURN_NONE;
    }
}
static int
Node_setparent(PyObject* myself, PyObject* value, void* closure)
{
    (void)myself; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "parent property is read-only");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_POSITION_DOC,"");
static PyObject*
Node_getposition(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return (PyObject*)vec3_ik_to_python(self->node->position.f);
}
static int
Node_setposition(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    if (!ik_Vec3_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Vec3() type for position");
        return -1;
    }
    ik_vec3_copy(self->node->position.f, ((ik_Vec3*)value)->vec.f);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_ROTATION_DOC,"");
static PyObject*
Node_getrotation(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return (PyObject*)quat_ik_to_python(self->node->rotation.f);
}
static int
Node_setrotation(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    if (!ik_Quat_CheckExact(value))
    {
        PyErr_SetString(PyExc_TypeError, "Expected a ik.Quat() type for rotation");
        return -1;
    }
    ik_quat_copy(self->node->rotation.f, ((ik_Quat*)value)->quat.f);
    return 0;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_MASS_DOC,"");
static PyObject*
Node_getmass(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return PyFloat_FromDouble(self->node->mass);
}
static int
Node_setmass(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)myself; (void)closure;

    if (PyFloat_Check(value))
    {
        self->node->mass = PyFloat_AS_DOUBLE(value);
        return 0;
    }
    if (PyLong_Check(value))
    {
        double d = PyLong_AsDouble(value);
        if (d == -1 && PyErr_Occurred())
            return -1;
        self->node->mass = d;
        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a float value");
    return -1;
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(NODE_ROTATION_WEIGHT_DOC,"");
static PyObject*
Node_getrotation_weight(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return PyFloat_FromDouble(self->node->rotation_weight);
}
static int
Node_setrotation_weight(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)myself; (void)closure;

    if (PyFloat_Check(value))
    {
        self->node->mass = PyFloat_AS_DOUBLE(value);
        return 0;
    }
    if (PyLong_Check(value))
    {
        double d = PyLong_AsDouble(value);
        if (d == -1 && PyErr_Occurred())
            return -1;
        self->node->mass = d;
        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a float value");
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
    {"count",           Node_getcount,           Node_setcount,           NODE_COUNT_DOC, NULL},
    {"child_count",     Node_getchild_count,     Node_setchild_count,     NODE_CHILD_COUNT_DOC, NULL},
    {"children",        Node_getchildren,        Node_setchildren,        NODE_CHILDREN_DOC, NULL},
    {"parent",          Node_getparent,          Node_setparent,          NODE_PARENT_DOC, NULL},
    {"position",        Node_getposition,        Node_setposition,        NODE_POSITION_DOC, NULL},
    {"rotation",        Node_getrotation,        Node_setrotation,        NODE_ROTATION_DOC, NULL},
    {"mass",            Node_getmass,            Node_setmass,            NODE_MASS_DOC, NULL},
    {"rotation_weight", Node_getrotation_weight, Node_setrotation_weight, NODE_ROTATION_WEIGHT_DOC, NULL},
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
static PyObject*
Node_richcompare(PyObject* myself, PyObject* other, int op)
{
    if (ik_Node_CheckExact(other))
    {
        ik_Node* self = (ik_Node*)myself;
        ik_Node* ikother = (ik_Node*)other;
        Py_RETURN_RICHCOMPARE(self->node->user.ptr, ikother->node->user.ptr, op);
    }
    else
    {
        Py_RETURN_NOTIMPLEMENTED;
    }
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
    .tp_init = (initproc)Node_init,
    .tp_richcompare = Node_richcompare
};

/* ------------------------------------------------------------------------- */
int
init_ik_NodeType(void)
{
    if (PyType_Ready(&ik_NodeType) < 0)
        return -1;
    if (PyType_Ready(&ik_NodeChildrenViewType) < 0)
        return -1;
    return 0;
}
