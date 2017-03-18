#ifndef BOARD_MODULE_H
#define BOARD_MODULE_H
#include <Python.h>
#include <structmember.h>
//#include <bool.h>
//#include "npy_common.h"
#include <numpy/arrayobject.h>
#include "gen.h"
#include "types.h"
#include "tile_definitions.h"
#include <stdlib.h>

#define MAX_QUEUE_SIZE 500
#define SQUARE_SIZE 32
#define COLOUR_OFFSET 4
#define COORD(x,y,z) (((x)*(y))+z)
#define XCOORD(o,ysize) ((o)/(ysize))
#define YCOORD(o,ysize) ((o)%(ysize))

typedef struct {
  PyObject_HEAD
  int score; //score of the current game
  int xsize,ysize;
  PyArrayObject *imgboard,*mineboard, *clickboard;
  npy_intp *imgdat, *minedat;
  board_t *mines,*clicks;
} boardPy;

//debug functions
static PyObject * _printclicks(boardPy *self);
static PyObject *_printmine(boardPy *self);

//specific functions
static PyObject * click (boardPy *self, PyObject *args);
static PyObject * score(boardPy *self, PyObject *args);
static PyObject * remake_wrapper(boardPy *self, PyObject *args);
static void remake (boardPy *self, PyObject *args);
static void build_image (boardPy *self);
//static void copy_image (char **map, char * image, int x, int y , int ysize);
static int permeate_click(boardPy *b, int x, int y);


static PyMethodDef  boardPy_methods[]={
  {"click",(PyCFunction)click,METH_VARARGS,"Method to implement clicking"},
  {"remake",(PyCFunction)remake_wrapper,METH_VARARGS,"Remake the board"},
  {"score",(PyCFunction)score,METH_NOARGS,"Gets the total score of the board"},
  {"_debugclicks",(PyCFunction)_printclicks,METH_NOARGS,"debug clicks board"},
  {"_debugmineboard",(PyCFunction)_printmine,METH_NOARGS,"debug nine board"},
  {NULL}
};

static PyMemberDef boardPy_members[]={
  {"score",T_INT,offsetof(boardPy,score),0,"current game score"},
  {"imgboard",T_OBJECT,offsetof(boardPy,imgboard),0,"current image"},
  {"mineboard",T_OBJECT,offsetof(boardPy,mineboard),0,"current mines"},
  {"clickboard",T_OBJECT,offsetof(boardPy,clickboard),0,"shows which fields are empty or full"},
  {NULL}
};




PyMODINIT_FUNC initboard(void);

//default functions
static void boardPy_dealloc(boardPy *self);
static int boardPy_init(boardPy *self, PyObject *args, PyObject *kwds);
static PyObject * boardPy_new (PyTypeObject *type, PyObject *args, PyObject *kwds);


static PyTypeObject boardPy_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "boardPy.BoardPy",             /* tp_name */
  sizeof(boardPy), /* tp_basicsize */
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


#endif

