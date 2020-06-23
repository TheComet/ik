#include "Python.h"

#define ik_Node_CheckExact(o) \
    (Py_TYPE(o) == &ik_NodeType)

struct ik_node;

typedef struct ik_NodeChildrenView
{
    PyObject_HEAD
    struct ik_node* node;
} ik_NodeChildrenView;

typedef struct ik_Node
{
    PyObject_HEAD

    struct ik_node* node;

    /* These can by Py_None */
    PyObject* algorithm;
    PyObject* constraint;
    PyObject* effector;
    PyObject* pole;

    /* Accesses node children */
    ik_NodeChildrenView* children;
} ik_Node;

extern PyTypeObject ik_NodeType;

int
init_ik_NodeType(void);
