#include "ik/python/ik_type_Algorithm.h"
#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_ModuleRef.h"
#include "ik/python/ik_type_Node.h"
#include "ik/python/ik_type_Pole.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_helpers.h"
#include "ik/python/ik_docstrings.h"
#include "ik/node.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"
#include "structmember.h"

/* ------------------------------------------------------------------------- */
static void
NodeChildrenView_dealloc(PyObject* myself)
{
    ik_NodeChildrenView* self = (ik_NodeChildrenView*)myself;

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
NodeChildrenView_repr_build_arglist_list(PyObject* myself)
{
    ik_NodeChildrenView* self = (ik_NodeChildrenView*)myself;
    PyObject* args = PyList_New(0);
    if (args == NULL)
        return NULL;

    NODE_FOR_EACH(self->node, user, child)
        int append_result;
        ik_Node* pychild = child->user.ptr;
        PyObject* arg = PyUnicode_FromFormat("%R", pychild);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    NODE_END_EACH

    return args;

    addarg_failed : Py_DECREF(args);
    return NULL;
}
static PyObject*
NodeChildrenView_repr_build_arglist_string(PyObject* myself, int* need_comma)
{
    PyObject* separator;
    PyObject* arglist;
    PyObject* string;

    separator = PyUnicode_FromString(", ");
    if (separator == NULL)
        return NULL;

    arglist = NodeChildrenView_repr_build_arglist_list(myself);
    if (arglist == NULL)
    {
        Py_DECREF(separator);
        return NULL;
    }

    *need_comma = (PyList_GET_SIZE(arglist) == 1);

    string = PyUnicode_Join(separator, arglist);
    Py_DECREF(separator);
    Py_DECREF(arglist);
    return string;
}

/* ------------------------------------------------------------------------- */
static PyObject*
NodeChildrenView_repr(PyObject* myself)
{
    int need_comma;
    PyObject* repr;
    PyObject* argstring = NodeChildrenView_repr_build_arglist_string(myself, &need_comma);
    if (argstring == NULL)
        return NULL;

    if (need_comma)
        repr = PyUnicode_FromFormat("(%U,)", argstring);
    else
        repr = PyUnicode_FromFormat("(%U)", argstring);
    Py_DECREF(argstring);
    return repr;
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

    node = ik_node_get_child(self->node, (uint32_t)index)->user.ptr;
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
PyTypeObject ik_NodeChildrenViewType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.NodeChildrenView",
    .tp_basicsize = sizeof(ik_NodeChildrenView),
    .tp_dealloc = NodeChildrenView_dealloc,
    .tp_repr = NodeChildrenView_repr,
    .tp_str = NodeChildrenView_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "",
    .tp_new = NodeChildrenView_new,
    .tp_as_mapping = &NodeChildrenView_as_mapping,
    .tp_as_sequence = &NodeChildrenView_as_sequence
};

/* ------------------------------------------------------------------------- */
static void
Node_dealloc(PyObject* myself)
{
    ik_Node* self = (ik_Node*)myself;

    /* Release references to all child python nodes */
    NODE_FOR_EACH(self->node, user, child)
        Py_DECREF(child->user.ptr);
    NODE_END_EACH
    IK_DECREF(self->node);

    UNREF_VEC3_DATA(self->position);
    UNREF_QUAT_DATA(self->rotation);
    Py_DECREF(self->position);
    Py_DECREF(self->rotation);

    Py_DECREF(self->algorithm);
    Py_DECREF(self->constraints);
    Py_DECREF(self->effector);
    Py_DECREF(self->pole);
    Py_DECREF(self->children);

    ik_NodeType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_Node* self;
    ik_NodeChildrenView* children_view;
    ik_Vec3* position;
    ik_Quat* rotation;
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

    /* Allocate other attributes */
    position = (ik_Vec3*)PyObject_CallObject((PyObject*)&ik_Vec3Type, NULL);
    if (position == NULL)
        goto alloc_position_failed;
    rotation = (ik_Quat*)PyObject_CallObject((PyObject*)&ik_QuatType, NULL);
    if (rotation == NULL)
        goto alloc_rotation_failed;

    /* Finally, alloc self */
    self = (ik_Node*)ik_NodeType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    /* Set up position/rotation */
    self->position = position;
    self->rotation = rotation;
    REF_VEC3_DATA(self->position, &node->position);
    REF_QUAT_DATA(self->rotation, &node->rotation);

    /* Set all attachments to None */
    Py_INCREF(Py_None); self->algorithm = Py_None;
    Py_INCREF(Py_None); self->constraints = Py_None;
    Py_INCREF(Py_None); self->effector = Py_None;
    Py_INCREF(Py_None); self->pole = Py_None;

    /*
     * Store the python object in node's user data so we don't have to store
     * child nodes in a python list.
     */
    node->user.ptr = self;

    /* store other objects we successfully allocated */
    self->node = node;
    self->children = children_view;

    return (PyObject*)self;

    alloc_self_failed             : Py_DECREF(rotation);
    alloc_rotation_failed         : Py_DECREF(position);
    alloc_position_failed         : Py_DECREF(children_view);
    alloc_children_view_failed    : IK_DECREF(node);
    alloc_node_failed             : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
Node_setconstraints(PyObject* myself, PyObject* value, void* closure);
static int
Node_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Algorithm* algorithm = NULL;
    PyObject* constraint_list = NULL;
    ik_Effector* effector = NULL;
    ik_Pole* pole = NULL;
    ik_Vec3* position = NULL;
    ik_Quat* rotation = NULL;
    ik_Node* self = (ik_Node*)myself;

    static char* kwds_str[] = {
        "position",
        "rotation",
        "algorithm",
        "constraints",
        "effector",
        "pole",
        "mass",
        "rotation_weight",
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|O!O!O!OO!O!" FMT FMT, kwds_str,
            &ik_Vec3Type, &position,
            &ik_QuatType, &rotation,
            &ik_AlgorithmType, &algorithm,
            &constraint_list,
            &ik_EffectorType, &effector,
            &ik_PoleType, &pole,
            &self->node->mass,
            &self->node->rotation_weight))
        return -1;

    if (constraint_list != NULL)
    {
        if (Node_setconstraints(myself, constraint_list, NULL) != 0)
            return -1;
    }

    if (algorithm != NULL)
    {
        PyObject* tmp;

        /* Attach to internal node */
        ik_node_attach_algorithm(self->node, (struct ik_algorithm*)algorithm->super.attachment);

        /* Set attachment on python object */
        tmp = (PyObject*)self->algorithm;
        Py_INCREF(algorithm);
        self->algorithm = (PyObject*)algorithm;
        Py_DECREF(tmp);
    }
    if (effector != NULL)
    {
        PyObject* tmp;

        /* Attach to internal node */
        ik_node_attach_effector(self->node, (struct ik_effector*)effector->super.attachment);

        /* Set attachment on python object */
        tmp = (PyObject*)self->effector;
        Py_INCREF(effector);
        self->effector = (PyObject*)effector;
        Py_DECREF(tmp);
    }
    if (pole != NULL)
    {
        PyObject* tmp;

        /* Attach to internal node */
        ik_node_attach_pole(self->node, (struct ik_pole*)pole->super.attachment);

        /* Set attachment on python object */
        tmp = (PyObject*)self->pole;
        Py_INCREF(pole);
        self->pole = (PyObject*)pole;
        Py_DECREF(tmp);
    }

    if (position != NULL)
        ASSIGN_VEC3(self->position, position);
    if (rotation != NULL)
        ASSIGN_QUAT(self->rotation, rotation);

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_link(PyObject* myself, PyObject* child)
{
    ik_Node* self = (ik_Node*)myself;

    if (!ik_Node_CheckExact(child))
    {
        PyErr_SetString(PyExc_TypeError, "Argument must be of type ik.Node");
        return NULL;
    }

    if (ik_node_link(self->node, ((ik_Node*)child)->node) != 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to link node (out of memory?) Check log for more info.");
        return NULL;
    }
    Py_INCREF(child);

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_unlink(PyObject* myself, PyObject* args)
{
    ik_Node* self = (ik_Node*)myself;
    (void)args;


    /* Parent is holding a reference to this python object, need to decref that
     * when unlinking */
    if (self->node->parent)
        Py_DECREF(self);

    ik_node_unlink(self->node);

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_create_child(PyObject* myself, PyObject* args, PyObject* kwds)
{
    PyObject* link_result;
    ik_Node* child;
    (void)args;

    child = (ik_Node*)PyObject_Call((PyObject*)&ik_NodeType, args, kwds);
    if (child == NULL)
        return NULL;

    link_result = Node_link(myself, (PyObject*)child);
    if (link_result == NULL)
    {
        Py_DECREF(child);
        return NULL;
    }
    Py_DECREF(link_result);

    return (PyObject*)child;
}

/* ------------------------------------------------------------------------- */
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
    {"link",         Node_link,                      METH_O,                       IK_NODE_LINK_DOC},
    {"unlink",       Node_unlink,                    METH_NOARGS,                  IK_NODE_UNLINK_DOC},
    {"create_child", (PyCFunction)Node_create_child, METH_VARARGS | METH_KEYWORDS, IK_NODE_CREATE_CHILD_DOC},
    {"pack",         Node_pack,                      METH_NOARGS,                  IK_NODE_PACK_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
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
static PyObject*
Node_getposition(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return Py_INCREF(self->position), (PyObject*)self->position;
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

    ASSIGN_VEC3(self->position, (ik_Vec3*)value);
    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_getrotation(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return Py_INCREF(self->rotation), (PyObject*)self->rotation;
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

    ASSIGN_QUAT(self->rotation, (ik_Quat*)value);
    return 0;
}

/* ------------------------------------------------------------------------- */
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
static PyObject*
Node_getalgorithm(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return Py_INCREF(self->algorithm), self->algorithm;
}
static int
Node_setalgorithm(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;

    if (ik_Algorithm_CheckExact(value))
    {
        PyObject* tmp;
        ik_Attachment* py_attachment = (ik_Attachment*)value;
        ik_node_attach_algorithm(self->node, (struct ik_algorithm*)py_attachment->attachment);

        tmp = self->algorithm;
        Py_INCREF(value);
        self->algorithm = value;
        Py_DECREF(tmp);

        return 0;
    }

    if (value == Py_None)
    {
        PyObject* tmp;
        ik_node_detach_algorithm(self->node);

        tmp = self->algorithm;
        Py_INCREF(Py_None);
        self->algorithm = Py_None;
        Py_DECREF(tmp);

        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Must assign an instance of type ik.Algorithm or None");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_getconstraints(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return Py_INCREF(self->constraints), (PyObject*)self->constraints;
}
static void
unlink_internal_constraints(ik_Node* self)
{
    int i;

    if (!PyTuple_CheckExact(self->constraints))
        return;

    for (i = 0; i != PyTuple_GET_SIZE(self->constraints); ++i)
    {
        ik_Constraint* py_constraint = (ik_Constraint*)PyTuple_GET_ITEM(self->constraints, i);
        struct ik_constraint* constraint = (struct ik_constraint*)py_constraint->super.attachment;
        constraint->next = NULL;
    }

    ik_node_detach_constraint(self->node);
}
static void
link_internal_constraints(ik_Node* self)
{
    int i;
    ik_Constraint* first_constraint = NULL;

    if (!PyTuple_CheckExact(self->constraints))
        return;

    for (i = 0; i != PyTuple_GET_SIZE(self->constraints); ++i)
    {
        ik_Constraint* py_constraint = (ik_Constraint*)PyTuple_GET_ITEM(self->constraints, i);
        struct ik_constraint* constraint = (struct ik_constraint*)py_constraint->super.attachment;

        if (i == 0)
            first_constraint = py_constraint;
        else
            ik_constraint_append((struct ik_constraint*)first_constraint->super.attachment, constraint);
    }

    if (first_constraint)
        ik_node_attach_constraint(self->node, (struct ik_constraint*)first_constraint->super.attachment);
}
static int
Node_setconstraints(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;

    if (PySequence_Check(value))
    {
        int i;
        PyObject* tmp;
        PyObject* seq = PySequence_Tuple(value);
        if (seq == NULL)
            return -1;

        /* unlinking internal constraints allows the same constraint objects to
         * be assigned in a different order */
        unlink_internal_constraints(self);

        /* If sequence is empty just assign None */
        if (PySequence_Fast_GET_SIZE(seq) == 0)
        {
            tmp = self->constraints;
            Py_INCREF(Py_None);
            self->constraints = Py_None;
            Py_DECREF(tmp);

            Py_DECREF(seq);
            return 0;
        }

        /* Check that all objects in the sequence are constraint instances and
         * aren't already linked to one another internally */
        for (i = 0; i != PySequence_Fast_GET_SIZE(seq); ++i)
        {
            ik_Constraint* py_constraint;
            struct ik_constraint* constraint;

            PyObject* item = PySequence_Fast_GET_ITEM(seq, i);
            if (!ik_Constraint_Check(item))
            {
                PyErr_Format(PyExc_TypeError, "Item %d in list is not an instance of ik.Constraint", i);
                Py_DECREF(seq);
                return -1;
            }

            py_constraint = (ik_Constraint*)item;
            constraint = (struct ik_constraint*)py_constraint->super.attachment;
            if (constraint->next)
            {
                PyErr_Format(PyExc_RuntimeError, "Constraint at index %d in list is already part of another constraint chain. Make sure you aren't assigning the same constraint object to multiple nodes, as this is not supported.", i);
                Py_DECREF(seq);
                return -1;
            }
        }

        /* Assign tuple as attachment */
        tmp = self->constraints;
        self->constraints = seq;
        link_internal_constraints(self);
        Py_DECREF(tmp);

        /* Finally, link all internal structures */

        return 0;
    }

    if (ik_Constraint_Check(value))
    {
        PyObject* tmp;
        PyObject* constraints = PyTuple_New(1);
        if (constraints == NULL)
            return -1;

        unlink_internal_constraints(self);
        Py_INCREF(value);
        PyTuple_SET_ITEM(constraints, 0, value);

        tmp = self->constraints;
        self->constraints = constraints;
        link_internal_constraints(self);
        Py_DECREF(tmp);

        return 0;
    }

    if (value == Py_None)
    {
        PyObject* tmp;
        unlink_internal_constraints(self);

        tmp = self->constraints;
        Py_INCREF(Py_None);
        self->constraints = Py_None;
        Py_DECREF(tmp);

        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Must assign an instance of type ik.Constraint, a list of ik.Constraint instances, or None");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_geteffector(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return Py_INCREF(self->effector), self->effector;
}
static int
Node_seteffector(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;

    if (ik_Effector_CheckExact(value))
    {
        PyObject* tmp;
        ik_Attachment* py_attachment = (ik_Attachment*)value;
        ik_node_attach_effector(self->node, (struct ik_effector*)py_attachment->attachment);

        tmp = self->effector;
        Py_INCREF(value);
        self->effector = value;
        Py_DECREF(tmp);

        return 0;
    }

    if (value == Py_None)
    {
        PyObject* tmp;
        ik_node_detach_effector(self->node);

        tmp = self->effector;
        Py_INCREF(Py_None);
        self->effector = Py_None;
        Py_DECREF(tmp);

        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Must assign an instance of type ik.Effector or None");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_getpole(PyObject* myself, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;
    return Py_INCREF(self->pole), self->pole;
}
static int
Node_setpole(PyObject* myself, PyObject* value, void* closure)
{
    ik_Node* self = (ik_Node*)myself;
    (void)closure;

    if (ik_Pole_CheckExact(value))
    {
        PyObject* tmp;
        ik_Attachment* py_attachment = (ik_Attachment*)value;
        ik_node_attach_pole(self->node, (struct ik_pole*)py_attachment->attachment);

        tmp = self->pole;
        Py_INCREF(value);
        self->pole = value;
        Py_DECREF(tmp);

        return 0;
    }

    if (value == Py_None)
    {
        PyObject* tmp;
        ik_node_detach_pole(self->node);

        tmp = self->pole;
        Py_INCREF(Py_None);
        self->pole = Py_None;
        Py_DECREF(tmp);

        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Must assign an instance of type ik.Pole or None");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Node_getset[] = {
    {"count",           Node_getcount,           Node_setcount,           IK_NODE_COUNT_DOC, NULL},
    {"child_count",     Node_getchild_count,     Node_setchild_count,     IK_NODE_CHILD_COUNT_DOC, NULL},
    {"children",        Node_getchildren,        Node_setchildren,        IK_NODE_CHILDREN_DOC, NULL},
    {"parent",          Node_getparent,          Node_setparent,          IK_NODE_PARENT_DOC, NULL},
    {"position",        Node_getposition,        Node_setposition,        IK_NODE_POSITION_DOC, NULL},
    {"rotation",        Node_getrotation,        Node_setrotation,        IK_NODE_ROTATION_DOC, NULL},
    {"mass",            Node_getmass,            Node_setmass,            IK_NODE_MASS_DOC, NULL},
    {"rotation_weight", Node_getrotation_weight, Node_setrotation_weight, IK_NODE_ROTATION_WEIGHT_DOC, NULL},
    {"algorithm",       Node_getalgorithm,       Node_setalgorithm,       IK_NODE_ALGORITHM_DOC, NULL},
    {"constraints",     Node_getconstraints,     Node_setconstraints,     IK_NODE_CONSTRAINTS_DOC, NULL},
    {"effector",        Node_geteffector,        Node_seteffector,        IK_NODE_EFFECTOR_DOC, NULL},
    {"pole",            Node_getpole,            Node_setpole,            IK_NODE_POLE_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Node_repr_build_arglist_list(PyObject* myself)
{
    ik_Node* self = (ik_Node*)myself;

    PyObject* args = PyList_New(0);
    if (args == NULL)
        return NULL;

    /* Attachments */
#define APPEND_ATTACHMENT(name)                                               \
    if (self->name != Py_None)                                                \
    {                                                                         \
        int append_result;                                                    \
        PyObject* arg = PyUnicode_FromFormat(#name "=%R", self->name);        \
        if (arg == NULL)                                                      \
            goto addarg_failed;                                               \
                                                                              \
        append_result = PyList_Append(args, arg);                             \
        Py_DECREF(arg);                                                       \
        if (append_result == -1)                                              \
            goto addarg_failed;                                               \
    }
    APPEND_ATTACHMENT(algorithm)
    APPEND_ATTACHMENT(constraints)
    APPEND_ATTACHMENT(effector)
    APPEND_ATTACHMENT(pole)
#undef APPEND_ATTACHMENT

    /* Position */
    {
        int append_result;
        PyObject* position;
        PyObject* arg;

        position = Node_getposition(myself, NULL);
        if (position == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("position=%R", position);
        Py_DECREF(position);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Rotation */
    {
        int append_result;
        PyObject* rotation;
        PyObject* arg;

        rotation = Node_getrotation(myself, NULL);
        if (rotation == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("rotation=%R", rotation);
        Py_DECREF(rotation);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Mass */
    if (self->node->mass != 1.0)
    {
        int append_result;
        PyObject* mass;
        PyObject* arg;

        mass = PyFloat_FromDouble(self->node->mass);
        if (mass == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("mass=%R", mass);
        Py_DECREF(mass);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Rotation weight */
    if (self->node->rotation_weight != 1.0)
    {
        int append_result;
        PyObject* rotation_weight;
        PyObject* arg;

        rotation_weight = PyFloat_FromDouble(self->node->rotation_weight);
        if (rotation_weight == NULL)
            goto addarg_failed;

        arg = PyUnicode_FromFormat("rotation_weight=%R", rotation_weight);
        Py_DECREF(rotation_weight);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    /* Child nodes */
    if (NodeChildrenView_length((PyObject*)self->children) > 0)
    {
        int append_result;
        PyObject* arg = PyUnicode_FromFormat("children=%R", (PyObject*)self->children);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    }

    return args;

    addarg_failed : Py_DECREF(args);
    return NULL;
}
static PyObject*
Node_repr_build_arglist_string(PyObject* myself)
{
    PyObject* separator;
    PyObject* arglist;
    PyObject* string;

    separator = PyUnicode_FromString(", ");
    if (separator == NULL)
        return NULL;

    arglist = Node_repr_build_arglist_list(myself);
    if (arglist == NULL)
    {
        Py_DECREF(separator);
        return NULL;
    }

    string = PyUnicode_Join(separator, arglist);
    Py_DECREF(separator);
    Py_DECREF(arglist);
    return string;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_repr(PyObject* myself)
{
    PyObject* repr;
    PyObject* argstring = Node_repr_build_arglist_string(myself);
    if (argstring == NULL)
        return NULL;

    repr = PyUnicode_FromFormat("ik.Node(%U)", argstring);
    Py_DECREF(argstring);
    return repr;
}

/* ------------------------------------------------------------------------- */
static PyObject*
Node_str(PyObject* myself)
{
    return Node_repr(myself);
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
PyTypeObject ik_NodeType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Node",
    .tp_basicsize = sizeof(ik_Node),
    .tp_dealloc = Node_dealloc,
    .tp_repr = Node_repr,
    .tp_str = Node_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_NODE_DOC,
    .tp_methods = Node_methods,
    .tp_getset = Node_getset,
    .tp_new = Node_new,
    .tp_init = Node_init,
    .tp_richcompare = Node_richcompare
};

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
init_ik_NodeType(void)
{
    ik_NodeType.tp_base = &ik_ModuleRefType;
    ik_PoseType.tp_base = &ik_ModuleRefType;

    if (PyType_Ready(&ik_NodeType) < 0)             return -1;
    if (PyType_Ready(&ik_PoseType) < 0)             return -1;
    if (PyType_Ready(&ik_NodeChildrenViewType) < 0) return -1;

    return 0;
}
