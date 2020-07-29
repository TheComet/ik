#pragma once

#include "ik/python/ik_type_ModuleRef.h"

#define ik_Node_CheckExact(o) \
    (Py_TYPE(o) == &ik_NodeType)

struct ik_node;
struct ik_pose;

struct ik_ConstraintList;
struct ik_Quat;
struct ik_Vec3;

typedef struct ik_NodeChildrenView
{
    PyObject_HEAD
    struct ik_node* node;
} ik_NodeChildrenView;

typedef struct ik_Node
{
    ik_ModuleRef super;

    struct ik_Vec3* position;
    struct ik_Quat* rotation;

    /* These can by Py_None */
    PyObject* algorithm;
    PyObject* constraints;
    PyObject* effector;
    PyObject* pole;

    /* Accesses node children */
    ik_NodeChildrenView* children;

    /* Internal node structure */
    struct ik_node* node;
} ik_Node;

typedef struct ik_Pose
{
    ik_ModuleRef super;
    struct ik_pose* pose;
} ik_Pose;

extern PyTypeObject ik_NodeType;
extern PyTypeObject ik_PoseType;

int
init_ik_NodeType(void);
