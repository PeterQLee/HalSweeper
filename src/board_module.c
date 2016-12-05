#include "board_module.h"


PyMODINIT_FUNC initboard(void) {
}


static void boardPy_dealloc(boardPy *self){
  destroy_board(self->mines);
  destroy_board(self->clicks);
  Py_DECREF(self->imgboard);

  Py_TYPE(self)->tp_free((PyObject*)self);

}
static int boardPy_init(boardPy *self, PyObject *args, PyObject *kwds){
  //static char *kwlist={"x","y"};
  self->imgboard=NULL;
  self->mines=malloc(sizeof(board_t));
  self->mines->board=NULL;
  self->clicks=malloc(sizeof(board_t));
  self->clicks->board=NULL;
  
}
static PyObject * boardPy_new (PyTypeObject *type, PyObject *args, PyObject *kwds){
  remake(self,args);
}

static void click (boardPy *self, PyObject *args){
  int x,y,o;
 
  PyObject *px=NULL, *py=NULL;
  if (!PyArg_ParseTuple(args,"ii",&px,&py)) return NULL;
  if (PyLong_Check(x))
    x=PyLong_AsLong(x)/SQUARE_SIZE;
  if (PyLong_Check(y))
    y=PyLong_AsLong(y)/SQUARE_SIZE;
  //todo:  make sure x and y are in range
  int bz=self->clicks->board[x*self->ysize+y];
  if (bz>0) return; //already clicked blank square
  if (bz & 0x10) {
    //game loss, do something
    return;//temp
  }
  else{
    self->clicks->board[x*self->ysize+y]=1;
    permeate_click(self,x,y);
    build_image(self);
  }
  
  
}
static void permeate_click(boardPy *b, int x, int y) {
  int o,lx,ly,index;
  //index=(x+(lx*o))*b->y + y+(ly*o)];
  queue_t Q;
  QPush(Q,COORD(x,b->ysize,y));
  while (Q.len>0){
    o=QPop(Q);
    b->clicks[o]=1;
    if (b->mines->board[o]==0 && b->clicks->board[o]==0){
      for (lx=-1;lx<=1;lx++){
	for (ly=-1;ly<=1;ly++){
	  if (x<b->xsize && x>0 && y<b->ysize && y>0) {
	    push(Q,COORD(x,b->ysize,y));
	  }
	}
      }
    }
  }
}
static PyObject * remake (boardPy *self, PyObject *args){
  //TODO: check errors
  static char *kwlist={"x","y"};
  PyObject *px=NULL, *py=NULL;

  int x,y;
  double keep_prob=0.5;
  
  if (!PyArg_ParseTuple(args,"ii",&px,&py)) return NULL;
  if (PyLong_Check(x))
    x=PyLong_AsLong(x);
  if (PyLong_Check(y))
    y=PyLong_AsLong(x);
  npy_intp dims[]={x,y};
  PyObject * img_npy=PyArray_SimpleNew(2,dims,PyArray_FILLWBYTE);
  if (self->imgboard)
    PyDECREF(self->imgboard);
  self->imgboard=img_npy;

  genboard(x,y,0,keep_prob,self->mines,self->clicks);
  
}

static void build_image(boardPy *self) {//builds pixel image
  self->imgboard->PyArray_DATA;
}


