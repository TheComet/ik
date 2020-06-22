#include "Python.h"

#define ik_Node_CheckExact(o) \
    (Py_TYPE(o) == &ik_NodeType)

struct ik_node;

typedef struct ik_Node
{
    PyObject_HEAD

    struct ik_node* node;

    /* These can by Py_None */
    PyObject* algorithm;
    PyObject* constraint;
    PyObject* effector;
    PyObject* pole;
} ik_Node;

extern PyTypeObject ik_NodeType;

int
init_ik_NodeType(void);
