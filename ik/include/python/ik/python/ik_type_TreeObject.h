#pragma once

#include "ik/python/ik_type_ModuleRef.h"

#define ik_TreeObject_CheckExact(o) \
    (Py_TYPE(o) == &ik_TreeObjectType)
#define ik_TreeObject_Check(o) \
    (PyObject_TypeCheck(o, &ik_TreeObjectType))

struct ik_tree_object;

typedef struct ik_TreeObjectChildrenView
{
    ik_ModuleRef super;
    struct ik_tree_object* tree_object;
} ik_TreeObjectChildrenView;

typedef struct ik_TreeObject
{
    ik_ModuleRef super;

    /* These can by Py_None */
    PyObject* algorithm;
    PyObject* constraints;
    PyObject* effector;
    PyObject* pole;

    /* Accesses tree_object children */
    ik_TreeObjectChildrenView* children;

    /* Internal tree_object structure */
    struct ik_tree_object* tree_object;
} ik_TreeObject;

extern PyTypeObject ik_TreeObjectType;

int
init_ik_TreeObjectType(void);

PyObject*
TreeObject_repr_build_arglist_list(PyObject* myself);
