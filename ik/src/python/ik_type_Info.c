#include "Python.h"
#include "ik/python/ik_type_Info.h"
#include "ik/info.h"

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_AUTHOR_DOC, "");
static PyObject*
Info_getauthor(PyObject* self, void* closure)
{
    (void)self; (void)closure;
    return PyUnicode_FromString(ik_info_author());
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_VERSION_DOC, "");
static PyObject*
Info_getversion(PyObject* self, void* closure)
{
    (void)self; (void)closure;
    return PyUnicode_FromString(ik_info_version());
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_BUILD_NUMBER_DOC, "");
static PyObject*
Info_getbuild_number(PyObject* self, void* closure)
{
    (void)self; (void)closure;
    return PyUnicode_FromFormat("%u", ik_info_build_number());
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_HOST_DOC, "");
static PyObject*
Info_gethost(PyObject* self, void* closure)
{
    (void)self; (void)closure;
    return PyUnicode_FromString(ik_info_host());
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_DATE_DOC, "");
static PyObject*
Info_getdate(PyObject* self, void* closure)
{
    (void)self; (void)closure;
    return PyUnicode_FromString(ik_info_date());
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_COMMIT_DOC, "");
static PyObject*
Info_getcommit(PyObject* self, void* closure)
{
    (void)self; (void)closure;
    return PyUnicode_FromString(ik_info_commit());
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_COMPILER_DOC, "");
static PyObject*
Info_getcompiler(PyObject* self, void* closure)
{
    (void)self; (void)closure;
    return PyUnicode_FromString(ik_info_compiler());
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_CMAKE_DOC, "");
static PyObject*
Info_getcmake(PyObject* self, void* closure)
{
    (void)self; (void)closure;
    return PyUnicode_FromString(ik_info_cmake());
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_ALL_DOC, "");
static PyObject*
Info_getall(PyObject* self, void* closure)
{
    (void)self; (void)closure;
    return PyUnicode_FromString(ik_info_all());
}

/* ------------------------------------------------------------------------- */
static PyGetSetDef Info_getset[] = {
    {"author",       Info_getauthor,       NULL, INFO_AUTHOR_DOC, NULL},
    {"version",      Info_getversion,      NULL, INFO_VERSION_DOC, NULL},
    {"build_number", Info_getbuild_number, NULL, INFO_BUILD_NUMBER_DOC, NULL},
    {"host",         Info_gethost,         NULL, INFO_HOST_DOC, NULL},
    {"date",         Info_getdate,         NULL, INFO_DATE_DOC, NULL},
    {"commit",       Info_getcommit,       NULL, INFO_COMMIT_DOC, NULL},
    {"compiler",     Info_getcompiler,     NULL, INFO_COMPILER_DOC, NULL},
    {"cmake",        Info_getcmake,        NULL, INFO_CMAKE_DOC, NULL},
    {"all",          Info_getall,          NULL, INFO_ALL_DOC, NULL},
    {NULL}
};

/* ------------------------------------------------------------------------- */
static PyObject*
Info_repr(PyObject* self)
{
    (void)self;
    return PyUnicode_FromString(ik_info_all());
}

/* ------------------------------------------------------------------------- */
static PyObject*
Info_str(PyObject* self)
{
    return Info_repr(self);
}

/* ------------------------------------------------------------------------- */
PyDoc_STRVAR(INFO_DOC, "");
PyTypeObject ik_InfoType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "ik.Info",
    .tp_basicsize = sizeof(PyObject),
    .tp_repr = Info_repr,
    .tp_str = Info_str,
    .tp_getset = Info_getset,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_doc = INFO_DOC
};

/* ------------------------------------------------------------------------- */
int
init_ik_InfoType(void)
{
    ik_InfoType.tp_new = PyType_GenericNew;
    if (PyType_Ready(&ik_InfoType) < 0)
        return -1;
    return 0;
}
