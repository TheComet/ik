#include "Python.h"

#define ik_Node_CheckExact(o) \
    (Py_TYPE(o) == &ik_NodeType)

struct ik_node;

typedef struct ik_Node
{
    PyObject_HEAD
    PyObject* user;
    struct ik_node* node;
    int guid;
} ik_Node;

extern PyTypeObject ik_NodeType;

int
init_ik_NodeType(void);
