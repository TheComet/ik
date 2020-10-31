#include "ik/python/ik_type_Algorithm.h"
#include "ik/python/ik_type_Constraint.h"
#include "ik/python/ik_type_Effector.h"
#include "ik/python/ik_type_TreeObject.h"
#include "ik/python/ik_type_Pole.h"
#include "ik/python/ik_type_Quat.h"
#include "ik/python/ik_type_Vec3.h"
#include "ik/python/ik_helpers.h"
#include "ik/python/ik_docstrings.h"
#include "ik/tree_object.h"
#include "ik/vec3.inl"
#include "ik/quat.inl"
#include "structmember.h"

extern PyTypeObject ik_TreeObjectChildrenViewType;

/* ------------------------------------------------------------------------- */
static void
TreeObjectChildrenView_dealloc(PyObject* myself)
{
    ik_TreeObjectChildrenView* self = (ik_TreeObjectChildrenView*)myself;

    IK_DECREF(self->tree_object);
    ik_TreeObjectChildrenViewType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObjectChildrenView_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_TreeObjectChildrenView* self;
    PyObject* tree_object_capsule;

    if (!PyArg_ParseTuple(args, "O!", &PyCapsule_Type, &tree_object_capsule))
        return NULL;

    self = (ik_TreeObjectChildrenView*)ik_TreeObjectChildrenViewType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    self->tree_object = PyCapsule_GetPointer(tree_object_capsule, NULL);
    IK_INCREF(self->tree_object);

    return (PyObject*)self;

    alloc_self_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObjectChildrenView_repr_build_arglist_list(PyObject* myself)
{
    ik_TreeObjectChildrenView* self = (ik_TreeObjectChildrenView*)myself;
    PyObject* args = PyList_New(0);
    if (args == NULL)
        return NULL;

    TREE_OBJECT_FOR_EACH_CHILD(self->tree_object, child)
        int append_result;
        ik_TreeObject* pychild = child->user_data;
        PyObject* arg = PyUnicode_FromFormat("%R", pychild);
        if (arg == NULL)
            goto addarg_failed;

        append_result = PyList_Append(args, arg);
        Py_DECREF(arg);
        if (append_result == -1)
            goto addarg_failed;
    TREE_OBJECT_END_EACH

    return args;

    addarg_failed : Py_DECREF(args);
    return NULL;
}
static PyObject*
TreeObjectChildrenView_repr_build_arglist_string(PyObject* myself, int* need_comma)
{
    PyObject* separator;
    PyObject* arglist;
    PyObject* string;

    separator = PyUnicode_FromString(", ");
    if (separator == NULL)
        return NULL;

    arglist = TreeObjectChildrenView_repr_build_arglist_list(myself);
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
TreeObjectChildrenView_repr(PyObject* myself)
{
    int need_comma;
    PyObject* repr;
    PyObject* argstring = TreeObjectChildrenView_repr_build_arglist_string(myself, &need_comma);
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
TreeObjectChildrenView_str(PyObject* myself)
{
    return TreeObjectChildrenView_repr(myself);
}

/* ------------------------------------------------------------------------- */
static Py_ssize_t
TreeObjectChildrenView_length(PyObject* myself)
{
    ik_TreeObjectChildrenView* self = (ik_TreeObjectChildrenView*)myself;

    return ik_tree_object_child_count(self->tree_object);
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObjectChildrenView_item(PyObject* myself, Py_ssize_t index)
{
    ik_TreeObject* tree_object;
    ik_TreeObjectChildrenView* self = (ik_TreeObjectChildrenView*)myself;

    if (index < 0 || index >= ik_tree_object_child_count(self->tree_object))
    {
        PyErr_SetString(PyExc_IndexError, "TreeObject child index out of range");
        return NULL;
    }

    tree_object = ik_tree_object_get_child(self->tree_object, (uint32_t)index)->user_data;
    return Py_INCREF(tree_object), (PyObject*)tree_object;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObjectChildrenView_subscript(PyObject* self, PyObject* item)
{
    if (PyIndex_Check(item))
    {
        Py_ssize_t idx = PyNumber_AsSsize_t(item, PyExc_IndexError);
        if (idx == -1 && PyErr_Occurred())
            return NULL;
        return TreeObjectChildrenView_item(self, idx);
    }
    else if (PySlice_Check(item))
    {
        PyErr_SetString(PyExc_TypeError, "TreeObject children can't be sliced (they're not ordered)");
        return NULL;
    }
    else
    {
        PyErr_SetString(PyExc_TypeError, "TreeObject child index must be an integer");
        return NULL;
    }
}

/* ------------------------------------------------------------------------- */
static PyMappingMethods TreeObjectChildrenView_as_mapping = {
    .mp_length = TreeObjectChildrenView_length,
    .mp_subscript = TreeObjectChildrenView_subscript
};

/* ------------------------------------------------------------------------- */
static PySequenceMethods TreeObjectChildrenView_as_sequence = {
    .sq_length = TreeObjectChildrenView_length,
    .sq_item = TreeObjectChildrenView_item
};

/* ------------------------------------------------------------------------- */
PyTypeObject ik_TreeObjectChildrenViewType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.TreeObjectChildrenView",
    .tp_basicsize = sizeof(ik_TreeObjectChildrenView),
    .tp_dealloc = TreeObjectChildrenView_dealloc,
    .tp_repr = TreeObjectChildrenView_repr,
    .tp_str = TreeObjectChildrenView_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = "",
    .tp_new = TreeObjectChildrenView_new,
    .tp_as_mapping = &TreeObjectChildrenView_as_mapping,
    .tp_as_sequence = &TreeObjectChildrenView_as_sequence
};

/* ------------------------------------------------------------------------- */
static void
TreeObject_dealloc(PyObject* myself)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;

    /* Release references to all child python tree_objects */
    TREE_OBJECT_FOR_EACH_CHILD(self->tree_object, child)
        Py_DECREF(child->user_data);
    TREE_OBJECT_END_EACH

    IK_DECREF(self->tree_object);

    Py_DECREF(self->algorithm);
    Py_DECREF(self->constraints);
    Py_DECREF(self->effector);
    Py_DECREF(self->pole);
    Py_DECREF(self->children);

    ik_TreeObjectType.tp_base->tp_dealloc(myself);
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_new(PyTypeObject* type, PyObject* args, PyObject* kwds)
{
    ik_TreeObject* self;
    ik_TreeObjectChildrenView* children_view;
    PyObject* capsule;
    PyObject* children_view_args;
    PyObject* base_args;

    /* First arg must be a capsule containing the internal bone */
    if (PyTuple_GET_SIZE(args) < 1)
    {
        PyErr_SetString(PyExc_TypeError, "Missing capsule");
        return NULL;
    }
    children_view_args = PyTuple_GetSlice(args, 0, 1);
    if (children_view_args == NULL)
        goto slice_children_view_args_failed;
    capsule = PyTuple_GET_ITEM(children_view_args, 0);
    if (!PyCapsule_CheckExact(capsule))
    {
        PyErr_SetString(PyExc_ValueError, "First argument must be a capsule");
        goto slice_base_args_failed;
    }

    /* Slice rest to get the base arglist */
    base_args = PyTuple_GetSlice(args, 1, PyTuple_GET_SIZE(args));
    if (base_args == NULL)
        goto slice_base_args_failed;

    /* create children view object */
    children_view = (ik_TreeObjectChildrenView*)PyObject_CallObject((PyObject*)&ik_TreeObjectChildrenViewType, children_view_args);
    if (children_view == NULL)
        goto alloc_children_view_failed;

    /* Finally, alloc self */
    self = (ik_TreeObject*)ik_TreeObjectType.tp_base->tp_new(type, args, kwds);
    if (self == NULL)
        goto alloc_self_failed;

    /* Set all attachments to None */
    Py_INCREF(Py_None); self->algorithm = Py_None;
    Py_INCREF(Py_None); self->constraints = Py_None;
    Py_INCREF(Py_None); self->effector = Py_None;
    Py_INCREF(Py_None); self->pole = Py_None;

    /* Store tree object from capsule */
    self->tree_object = PyCapsule_GetPointer(capsule, NULL);
    IK_INCREF(self->tree_object);

    /* Store children view */
    self->children = children_view;

    /*
     * Store the python object in tree_object's user data so we don't have to store
     * child tree_objects in a python list.
     */
    self->tree_object->user_data = self;

    /* Clean up */
    Py_DECREF(base_args);
    Py_DECREF(children_view_args);

    return (PyObject*)self;

    alloc_self_failed               : Py_DECREF(children_view);
    alloc_children_view_failed      : Py_DECREF(base_args);
    slice_base_args_failed          : Py_DECREF(children_view_args);
    slice_children_view_args_failed : return NULL;
}

/* ------------------------------------------------------------------------- */
static int
TreeObject_setchildren(PyObject* myself, PyObject* value, void* closure);
static int
TreeObject_setconstraints(PyObject* myself, PyObject* value, void* closure);
static int
TreeObject_init(PyObject* myself, PyObject* args, PyObject* kwds)
{
    ik_Algorithm* algorithm = NULL;
    PyObject* constraint_list = NULL;
    PyObject* children_list = NULL;
    ik_Effector* effector = NULL;
    ik_Pole* pole = NULL;
    ik_Vec3* position = NULL;
    ik_Quat* rotation = NULL;
    ik_TreeObject* self = (ik_TreeObject*)myself;

    static char* kwds_str[] = {
        "children",
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

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "|OO!O!O!OO!O!" FMT FMT, kwds_str,
            &children_list,
            &ik_Vec3Type, &position,
            &ik_QuatType, &rotation,
            &ik_AlgorithmType, &algorithm,
            &constraint_list,
            &ik_EffectorType, &effector,
            &ik_PoleType, &pole,
            &self->tree_object->mass,
            &self->tree_object->rotation_weight))
        return -1;

    if (children_list != NULL)
    {
        if (TreeObject_setchildren(myself, children_list, NULL) != 0)
            return -1;
    }

    if (constraint_list != NULL)
    {
        if (TreeObject_setconstraints(myself, constraint_list, NULL) != 0)
            return -1;
    }

    if (algorithm != NULL)
    {
        PyObject* tmp;

        /* Attach to internal tree_object */
        ik_tree_object_attach_algorithm(self->tree_object, (struct ik_algorithm*)algorithm->super.attachment);

        /* Set attachment on python object */
        tmp = (PyObject*)self->algorithm;
        Py_INCREF(algorithm);
        self->algorithm = (PyObject*)algorithm;
        Py_DECREF(tmp);
    }
    if (effector != NULL)
    {
        PyObject* tmp;

        /* Attach to internal tree_object */
        ik_tree_object_attach_effector(self->tree_object, (struct ik_effector*)effector->super.attachment);

        /* Set attachment on python object */
        tmp = (PyObject*)self->effector;
        Py_INCREF(effector);
        self->effector = (PyObject*)effector;
        Py_DECREF(tmp);
    }
    if (pole != NULL)
    {
        PyObject* tmp;

        /* Attach to internal tree_object */
        ik_tree_object_attach_pole(self->tree_object, (struct ik_pole*)pole->super.attachment);

        /* Set attachment on python object */
        tmp = (PyObject*)self->pole;
        Py_INCREF(pole);
        self->pole = (PyObject*)pole;
        Py_DECREF(tmp);
    }

    return 0;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_link(PyObject* myself, PyObject* child)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;

    if (!ik_TreeObject_Check(child))
    {
        PyErr_SetString(PyExc_TypeError, "Argument must be of type ik.TreeObject");
        return NULL;
    }

    if (ik_tree_object_link(self->tree_object, ((ik_TreeObject*)child)->tree_object) != 0)
    {
        PyErr_SetString(PyExc_RuntimeError, "Failed to link tree_object. Check log for more info.");
        return NULL;
    }
    Py_INCREF(child);

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_unlink(PyObject* myself, PyObject* args)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)args;

    /* Parent is holding a reference to this python object, need to decref that
     * when unlinking */
    if (self->tree_object->parent)
        Py_DECREF(self);

    ik_tree_object_unlink(self->tree_object);

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_unlink_all_children(PyObject* myself, PyObject* args)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)args;

    TREE_OBJECT_FOR_EACH_CHILD(self->tree_object, child)
        Py_DECREF(child->user_data);
    TREE_OBJECT_END_EACH
    ik_tree_object_unlink_all_children(self->tree_object);

    Py_RETURN_NONE;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_create_child(PyObject* myself, PyObject* args, PyObject* kwds)
{
    PyObject* link_result;
    ik_TreeObject* child;
    (void)args;

    child = (ik_TreeObject*)PyObject_Call((PyObject*)Py_TYPE(myself), args, kwds);
    if (child == NULL)
        return NULL;

    link_result = TreeObject_link(myself, (PyObject*)child);
    if (link_result == NULL)
    {
        Py_DECREF(child);
        return NULL;
    }
    Py_DECREF(link_result);

    return (PyObject*)child;
}

/* ------------------------------------------------------------------------- */
static PyMethodDef TreeObject_methods[] = {
    {"link",                TreeObject_link,                      METH_O,                       IK_TREE_OBJECT_LINK_DOC},
    {"unlink",              TreeObject_unlink,                    METH_NOARGS,                  IK_TREE_OBJECT_UNLINK_DOC},
    {"unlink_all_children", TreeObject_unlink_all_children,       METH_NOARGS,                  IK_TREE_OBJECT_UNLINK_ALL_CHILDREN_DOC},
    {"create_child",        (PyCFunction)TreeObject_create_child, METH_VARARGS | METH_KEYWORDS, IK_TREE_OBJECT_CREATE_CHILD_DOC},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_getcount(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;
    return PyLong_FromLong(ik_tree_object_count(self->tree_object));
}
static int
TreeObject_setcount(PyObject* myself, PyObject* value, void* closure)
{
    (void)myself; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "Count is read-only");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_getchild_count(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;
    return PyLong_FromLong(ik_tree_object_child_count(self->tree_object));
}
static int
TreeObject_setchild_count(PyObject* myself, PyObject* value, void* closure)
{
    (void)myself; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "Child count is read-only");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_getchildren(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;
    return Py_INCREF(self->children), (PyObject*)self->children;
}
static int
TreeObject_setchildren(PyObject* myself, PyObject* value, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;

    /* Children can be a single ik_TreeObject instance, or a list of ik_TreeObject */
    if (ik_TreeObject_Check(value))
    {
        struct cs_vector old_children;
        PyObject* result;
        ik_TreeObject* new_child = (ik_TreeObject*)value;

        old_children = self->tree_object->children;
        vector_init(&self->tree_object->children, sizeof(struct ik_tree_object*));

        if (!ik_tree_object_can_link(self->tree_object, new_child->tree_object))
        {
            PyErr_SetString(PyExc_RuntimeError, "Can't link tree_object because it would create a circular dependency");
            goto restore_old_children1;
        }

        new_child->tree_object->parent = NULL;
        result = TreeObject_link(myself, value);
        if (result == NULL)
            goto restore_old_children1;
        Py_DECREF(result);

        /* Unlink old children */
        VECTOR_FOR_EACH(&old_children, struct ik_tree_object*, pchild)
            struct ik_tree_object* child = *pchild;
            if (child->parent != (struct ik_tree_object*)self->tree_object)
                child->parent = NULL;
            Py_DECREF(child->user_data);
            IK_DECREF(child);
        VECTOR_END_EACH
        vector_deinit(&old_children);

        return 0;

        restore_old_children1 : vector_deinit(&self->tree_object->children);
                                self->tree_object->children = old_children;
                                TREE_OBJECT_FOR_EACH_CHILD(self->tree_object, child)
                                    child->parent = (struct ik_tree_object*)self->tree_object;
                                TREE_OBJECT_END_EACH
        return -1;
    }
    else if (PySequence_Check(value))
    {
        int i;
        struct cs_vector old_children;
        PyObject* seq = PySequence_Tuple(value);
        if (seq == NULL)
            return -1;

        old_children = self->tree_object->children;
        vector_init(&self->tree_object->children, sizeof(struct ik_tree_object*));

        for (i = 0; i != PySequence_Fast_GET_SIZE(seq); ++i)
        {
            PyObject* result;
            PyObject* child = PySequence_Fast_GET_ITEM(seq, i);
            if (!ik_TreeObject_Check(child))
            {
                PyErr_Format(PyExc_TypeError, "Object at index %d is not of type ik.TreeObject", i);
                goto restore_old_children2;
            }

            if (!ik_tree_object_can_link(self->tree_object, ((ik_TreeObject*)child)->tree_object))
            {
                PyErr_Format(PyExc_RuntimeError, "Can't link tree_object at index %d because it would create a circular dependency", i);
                goto restore_old_children2;
            }

            ((ik_TreeObject*)child)->tree_object->parent = NULL;
            result = TreeObject_link(myself, child);
            if (result == NULL)
                goto restore_old_children2;
            Py_DECREF(result);
        }

        /* unlink old children */
        VECTOR_FOR_EACH(&old_children, struct ik_tree_object*, pchild)
            struct ik_tree_object* child = *pchild;
            if (child->parent != (struct ik_tree_object*)self->tree_object)
                child->parent = NULL;
            Py_DECREF(child->user_data);
            IK_DECREF(child);
        VECTOR_END_EACH
        vector_deinit(&old_children);
        Py_DECREF(seq);

        return 0;

        restore_old_children2 : Py_DECREF(TreeObject_unlink_all_children(myself, NULL));
                                vector_deinit(&self->tree_object->children);
                                self->tree_object->children = old_children;
                                TREE_OBJECT_FOR_EACH_CHILD(self->tree_object, child)
                                    child->parent = (struct ik_tree_object*)self->tree_object;
                                TREE_OBJECT_END_EACH
                                Py_DECREF(seq);
        return -1;
    }
    else if (value == Py_None)
    {
        Py_DECREF(TreeObject_unlink_all_children(myself, NULL));
        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a list of ik.TreeObject objects or a single instance of ik.TreeObject or None");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_getparent(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;

    if (self->tree_object->parent != NULL)
    {
        ik_TreeObject* parent = self->tree_object->parent->user_data;
        return Py_INCREF(parent), (PyObject*)parent;
    }
    else
    {
        Py_RETURN_NONE;
    }
}
static int
TreeObject_setparent(PyObject* myself, PyObject* value, void* closure)
{
    (void)myself; (void)value; (void)closure;
    PyErr_SetString(PyExc_AttributeError, "parent property is read-only");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_getmass(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;
    return PyFloat_FromDouble(self->tree_object->mass);
}
static int
TreeObject_setmass(PyObject* myself, PyObject* value, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)myself; (void)closure;

    if (PyFloat_Check(value))
    {
        self->tree_object->mass = PyFloat_AS_DOUBLE(value);
        return 0;
    }
    if (PyLong_Check(value))
    {
        double d = PyLong_AsDouble(value);
        if (d == -1 && PyErr_Occurred())
            return -1;
        self->tree_object->mass = d;
        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a float value");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_getrotation_weight(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;
    return PyFloat_FromDouble(self->tree_object->rotation_weight);
}
static int
TreeObject_setrotation_weight(PyObject* myself, PyObject* value, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)myself; (void)closure;

    if (PyFloat_Check(value))
    {
        self->tree_object->mass = PyFloat_AS_DOUBLE(value);
        return 0;
    }
    if (PyLong_Check(value))
    {
        double d = PyLong_AsDouble(value);
        if (d == -1 && PyErr_Occurred())
            return -1;
        self->tree_object->mass = d;
        return 0;
    }

    PyErr_SetString(PyExc_TypeError, "Expected a float value");
    return -1;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_getalgorithm(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;
    return Py_INCREF(self->algorithm), self->algorithm;
}
static int
TreeObject_setalgorithm(PyObject* myself, PyObject* value, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;

    if (ik_Algorithm_CheckExact(value))
    {
        PyObject* tmp;
        ik_Attachment* py_attachment = (ik_Attachment*)value;
        ik_tree_object_attach_algorithm(self->tree_object, (struct ik_algorithm*)py_attachment->attachment);

        tmp = self->algorithm;
        Py_INCREF(value);
        self->algorithm = value;
        Py_DECREF(tmp);

        return 0;
    }

    if (value == Py_None)
    {
        PyObject* tmp;
        ik_tree_object_detach_algorithm(self->tree_object);

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
TreeObject_getconstraints(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;
    return Py_INCREF(self->constraints), (PyObject*)self->constraints;
}
static void
unlink_internal_constraints(ik_TreeObject* self)
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

    ik_tree_object_detach_constraint(self->tree_object);
}
static void
link_internal_constraints(ik_TreeObject* self)
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
        ik_tree_object_attach_constraint(self->tree_object, (struct ik_constraint*)first_constraint->super.attachment);
}
static int
TreeObject_setconstraints(PyObject* myself, PyObject* value, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
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
                PyErr_Format(PyExc_RuntimeError, "Constraint at index %d in list is already part of another constraint chain. Make sure you aren't assigning the same constraint object to multiple tree_objects, as this is not supported.", i);
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
TreeObject_geteffector(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;
    return Py_INCREF(self->effector), self->effector;
}
static int
TreeObject_seteffector(PyObject* myself, PyObject* value, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;

    if (ik_Effector_CheckExact(value))
    {
        PyObject* tmp;
        ik_Attachment* py_attachment = (ik_Attachment*)value;
        ik_tree_object_attach_effector(self->tree_object, (struct ik_effector*)py_attachment->attachment);

        tmp = self->effector;
        Py_INCREF(value);
        self->effector = value;
        Py_DECREF(tmp);

        return 0;
    }

    if (value == Py_None)
    {
        PyObject* tmp;
        ik_tree_object_detach_effector(self->tree_object);

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
TreeObject_getpole(PyObject* myself, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;
    return Py_INCREF(self->pole), self->pole;
}
static int
TreeObject_setpole(PyObject* myself, PyObject* value, void* closure)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;
    (void)closure;

    if (ik_Pole_CheckExact(value))
    {
        PyObject* tmp;
        ik_Attachment* py_attachment = (ik_Attachment*)value;
        ik_tree_object_attach_pole(self->tree_object, (struct ik_pole*)py_attachment->attachment);

        tmp = self->pole;
        Py_INCREF(value);
        self->pole = value;
        Py_DECREF(tmp);

        return 0;
    }

    if (value == Py_None)
    {
        PyObject* tmp;
        ik_tree_object_detach_pole(self->tree_object);

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
static PyGetSetDef TreeObject_getset[] = {
    {"count",            TreeObject_getcount,            TreeObject_setcount,            IK_TREE_OBJECT_COUNT_DOC, NULL},
    {"child_count",      TreeObject_getchild_count,      TreeObject_setchild_count,      IK_TREE_OBJECT_CHILD_COUNT_DOC, NULL},
    {"children",         TreeObject_getchildren,         TreeObject_setchildren,         IK_TREE_OBJECT_CHILDREN_DOC, NULL},
    {"parent",           TreeObject_getparent,           TreeObject_setparent,           IK_TREE_OBJECT_PARENT_DOC, NULL},
    {"mass",             TreeObject_getmass,             TreeObject_setmass,             IK_TREE_OBJECT_MASS_DOC, NULL},
    {"rotation_weight",  TreeObject_getrotation_weight,  TreeObject_setrotation_weight,  IK_TREE_OBJECT_ROTATION_WEIGHT_DOC, NULL},
    {"algorithm",        TreeObject_getalgorithm,        TreeObject_setalgorithm,        IK_TREE_OBJECT_ALGORITHM_DOC, NULL},
    {"constraints",      TreeObject_getconstraints,      TreeObject_setconstraints,      IK_TREE_OBJECT_CONSTRAINTS_DOC, NULL},
    {"effector",         TreeObject_geteffector,         TreeObject_seteffector,         IK_TREE_OBJECT_EFFECTOR_DOC, NULL},
    {"pole",             TreeObject_getpole,             TreeObject_setpole,             IK_TREE_OBJECT_POLE_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
PyObject*
TreeObject_repr_build_arglist_list(PyObject* myself)
{
    ik_TreeObject* self = (ik_TreeObject*)myself;

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

    /* Mass */
    if (self->tree_object->mass != 1.0)
    {
        int append_result;
        PyObject* mass;
        PyObject* arg;

        mass = PyFloat_FromDouble(self->tree_object->mass);
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
    if (self->tree_object->rotation_weight != 1.0)
    {
        int append_result;
        PyObject* rotation_weight;
        PyObject* arg;

        rotation_weight = PyFloat_FromDouble(self->tree_object->rotation_weight);
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

    /* Child tree_objects */
    if (TreeObjectChildrenView_length((PyObject*)self->children) > 0)
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
TreeObject_repr_build_arglist_string(PyObject* myself)
{
    PyObject* separator;
    PyObject* arglist;
    PyObject* string;

    separator = PyUnicode_FromString(", ");
    if (separator == NULL)
        return NULL;

    arglist = TreeObject_repr_build_arglist_list(myself);
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
TreeObject_repr(PyObject* myself)
{
    PyObject* repr;
    PyObject* argstring = TreeObject_repr_build_arglist_string(myself);
    if (argstring == NULL)
        return NULL;

    repr = PyUnicode_FromFormat("ik.TreeObject(%U)", argstring);
    Py_DECREF(argstring);
    return repr;
}

/* ------------------------------------------------------------------------- */
static PyObject*
TreeObject_str(PyObject* myself)
{
    return TreeObject_repr(myself);
}

/* ------------------------------------------------------------------------- */
/*static PyObject*
TreeObject_richcompare(PyObject* myself, PyObject* other, int op)
{
    if (ik_TreeObject_CheckExact(other))
    {
        ik_TreeObject* self = (ik_TreeObject*)myself;
        ik_TreeObject* ikother = (ik_TreeObject*)other;
        Py_RETURN_RICHCOMPARE(self->tree_object->user_data, ikother->tree_object->user_data, op);
    }
    else
    {
        Py_RETURN_NOTIMPLEMENTED;
    }
}*/

/* ------------------------------------------------------------------------- */
PyTypeObject ik_TreeObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.TreeObject",
    .tp_basicsize = sizeof(ik_TreeObject),
    .tp_dealloc = TreeObject_dealloc,
    .tp_repr = TreeObject_repr,
    .tp_str = TreeObject_str,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = IK_TREE_OBJECT_DOC,
    .tp_methods = TreeObject_methods,
    .tp_getset = TreeObject_getset,
    .tp_new = TreeObject_new,
    .tp_init = TreeObject_init
    /*.tp_richcompare = TreeObject_richcompare*/
};

/* ------------------------------------------------------------------------- */
int
init_ik_TreeObjectType(void)
{
    ik_TreeObjectType.tp_base = &ik_ModuleRefType;
    ik_TreeObjectChildrenViewType.tp_base = &ik_ModuleRefType;

    if (PyType_Ready(&ik_TreeObjectType) < 0)             return -1;
    if (PyType_Ready(&ik_TreeObjectChildrenViewType) < 0) return -1;

    return 0;
}
