#include "Python.h"

struct ik_node_t;

typedef struct ik_Node
{
    PyObject_HEAD
    struct ik_node_t* node;
} ik_Node;

extern PyTypeObject ik_NodeType;

int
init_ik_NodeType(void);
