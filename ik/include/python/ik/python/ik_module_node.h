#include "Python.h"
#include "ik/node.h"

typedef struct ik_Node
{
    PyObject_HEAD
    struct ik_node_t node;
} ik_Node;

extern PyTypeObject ik_NodeType;

int
init_ik_NodeType(void);
