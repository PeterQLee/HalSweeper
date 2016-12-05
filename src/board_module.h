#ifndef BOARD_MODULE_H
#define BOARD_MODULE_H
#include <Python.h>
#include <structmember.h>
#include <bool.h>
#include "gen.h"

#define SQUARE_SIZE 30
#define COORD(x,y,z): ((x)*(y))+z

typedef struct {
  PyObject_HEAD
  int score; //score of the current game
  int xsize,ysize;
  PyArrayObject * imgboard;
  board_t *mines,*clicks;
} boardPy;

static PyMethodDef  boardPy_methods[]={
  {"click",click,METH_VARARGS,"Method to implement clicking"},
  {"remake",remake,METH_VARARGS,"Remake the board"}
};

static PyMemberDef boardPy_members[]={
  {"score",T_INT,offsetof(boardPy,score),0,"current game score"}
};

PyMODINIT_FUNC initboard(void);

//default functions
static void boardPy_dealloc(boardPy *self);
static int boardPy_init(boardPy *self, PyObject *args, PyObject *kwds);
static PyObject * boardPy_new (PyTypeObject *type, PyObject *args, PyObject *kwds);

static PyTypeObject boardPy_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "test.Test",             /* tp_name */
  sizeof(test_TestObject), /* tp_basicsize */
  0,                         /* tp_itemsize */
  boardPy_dealloc,                         /* tp_dealloc */
  0,                         /* tp_print */
  0,                         /* tp_getattr */
  0,                         /* tp_setattr */
  0,                         /* tp_as_async */
  0,                         /* tp_repr */
  0,                         /* tp_as_number */
  0,                         /* tp_as_sequence */
  0,                         /* tp_as_mapping */
  0,                         /* tp_hash  */
  0,                         /* tp_call */
  0,                         /* tp_str */
  0,                         /* tp_getattro */
  0,                         /* tp_setattro */
  0,                         /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT|Py_TPFLAGS_BASETYPE,        /* tp_flags */
  "Minesweeper Board object",           /* tp_doc */
  0,                         /* tp_traverse */
  0,                         /* tp_clear */
  0,                         /* tp_richcompare */
  0,                         /* tp_weaklistoffset */
  0,                         /* tp_iter */
  0,                         /* tp_iternext */
  boardPy_methods,             /* tp_methods */
  boardPy_members,             /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  (initproc)boardPy_init,      /* tp_init */
  0,                         /* tp_alloc */
  boardPy_new,                 /* tp_new */
};

static PyModuleDef boardPymodule = {
  PyModuleDef_HEAD_INIT,
  "BoardPy",
  "TBH",
  -1,
  NULL, NULL, NULL, NULL, NULL
};

//specific functions
static PyObject * click (boardPy *self, PyObject *args);
static PyObject * remake (boardPy *self, PyObject *args);
static void build_image (boardPy *self);


#endif

