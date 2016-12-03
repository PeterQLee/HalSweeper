#include "board_module.h"


PyMODINIT_FUNC initboard(void) {
}


static void boardPy_dealloc(boardPy *self){
  Py_TYPE(self)->tp_free((PyObject*)self);
}
static int boardPy_init(boardPy *self, PyObject *args, PyObject *kwds){
  static char *kwlist={"x","y"};
  self->imgboard=NULL;
  self->mines=NULL;
  self->clicks=NULL;
  remake(self,args);
}
static PyObject * boardPy_new (PyTypeObject *type, PyObject *args, PyObject *kwds){
  
}

static PyObject * click (PyObject *self, PyObject *args){
  
}
static PyObject * remake (PyObject *self, PyObject *args){
  //TODO: check errors
  static char *kwlist={"x","y"};
  PyObject *x=NULL, *y=NULL;
  if (!PyArg_ParseTuple(args,"ii",&x,&y)) return NULL;
  if (PyLong_Check(x))
    x=PyLong_AsLong(x);
  if (PyLong_Check(y))
    y=PyLong_AsLong(x);
  npy_intp dims[]={x,y};
  PyObject * img_npy=PyArray_SimpleNew(2,dims,PyArray_FILLWBYTE);
  if (self->imgboard)
    PyDECREF(self->imgboard);
  self->imgboard=img_npy;
  
}



