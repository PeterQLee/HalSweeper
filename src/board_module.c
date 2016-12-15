#include "board_module.h"
//debug functions

static PyObject * _printclicks(boardPy *self){
  debug_printboard(self->clicks);
  return Py_BuildValue("");
}
static PyObject * _printmine(boardPy *self){
  debug_printboard(self->mines);
  return Py_BuildValue("");
}

static void boardPy_dealloc(boardPy *self){
  
  destroy_board(self->mines);
  destroy_board(self->clicks);
  if (self->imgboard) //temp
    Py_DECREF(self->imgboard);
  Py_TYPE(self)->tp_free((PyObject*)self);
  printf("DEALLOC\n");
}
static int boardPy_init(boardPy *self, PyObject *args, PyObject *kwds){
  self->imgboard=NULL;
  self->mines=malloc(sizeof(board_t));
  self->mines->board=NULL;
  self->clicks=malloc(sizeof(board_t));
  self->clicks->board=NULL;
  remake(self,args);
  printf ("DONE INIT\n");
  return 0;
}
static PyObject * boardPy_new (PyTypeObject *type, PyObject *args, PyObject *kwds){
  boardPy *self;
  self=(boardPy*)type->tp_alloc(type,0);
  return (PyObject *)self;
}
static PyObject * click (boardPy *self, PyObject *args){
  int x,y;
 
  //PyObject px, py;
  if (!PyArg_ParseTuple(args,"ii",&x,&y)) return NULL;
  //printf ("AYY %p",&px);
  /* if (PyLong_Check(&px)) */
  /*   x=PyLong_AsLong(&px)/SQUARE_SIZE; */
  /* if (PyLong_Check(&py)) */
  /*   y=PyLong_AsLong(&py)/SQUARE_SIZE; */
  //todo:  make sure x and y are in range
  int bz=self->clicks->board[x*(self->ysize)+y];
  if (bz>0) return NULL; //already clicked blank square
  if (bz & 0x10) {
    //game loss, do something
    return NULL;//temp
  }
  else{
    printf("AGG %d\n",x*(self->ysize)+y);
    permeate_click(self,x,y);
    build_image(self);
  }
  return self->imgboard;
  
  
}
static int permeate_click(boardPy *b, int x, int y) {
  int lx,ly,index,_m;
  long o,px,py;
  printf("CLICK AT %d %d\n",x,y);
  //index=(x+(lx*o))*b->y + y+(ly*o)];
  queue_t * Q=create_queue(MAX_QUEUE_SIZE);
  push_Q(Q,(long)COORD(x,b->ysize,y));
  while (Q->cursize>0){
    o=pop_Q(Q,&_m);
    if (_m) printf("OOOH NOO\n");
    printf("%ld o",o);
    px=XCOORD(o,b->ysize);
    py=YCOORD(o,b->ysize);

    if (b->mines->board[o]==0 && b->clicks->board[o]==0){
      for (lx=-1;lx<=1;lx++){
  	for (ly=-1;ly<=1;ly++){
	  printf("Next %d %d\n",px+lx,py+ly);
  	  if (px+lx<(b->xsize) && px+lx>0 && py+ly<(b->ysize) && py+ly>0) {
  	    if( push_Q(Q,(long)COORD(px+lx,b->ysize,py+ly))) {printf("AHHH\n");}
	    printf("Next %d %d\n",px+lx,py+ly);
  	  }
  	}
      }
    }
    b->clicks->board[o]=1;
  }
  return 0;
}
static void remake (boardPy *self, PyObject *args){
  //TODO: check errors
  //static char kwlist[2]={'x','y'};
  //PyObject *px=NULL, *py=NULL;
  int i;
  npy_intp x,y;
  double keep_prob=0.3;
  
  if (!PyArg_ParseTuple(args,"ii",&x,&y)) return;
  
  /* if (PyLong_Check(x)) */
  /*   x=PyLong_AsLong(x); */
  /* if (PyLong_Check(y)) */
  /*   y=PyLong_AsLong(x); */
  npy_intp dims[2]={x,y};
  //PyObject * img_npy=PyArray_ZEROS(2,dims,NPY_LONG,0);
  npy_intp *dat=calloc(sizeof(npy_intp),x*y);

  PyObject * img_npy=PyArray_SimpleNewFromData(2,dims,NPY_LONG,dat);
  if (self->imgboard)
    Py_DECREF(self->imgboard);

  self->imgboard=(PyArrayObject *)img_npy;

  genboard(x,y,0,keep_prob,self->mines,self->clicks);  //#
  self->ysize=y;
  self->xsize=x;
  printf( "MADE\n");
  //return self->imgboard;
}

static void build_image(boardPy *self) {//builds pixel image
  //self->imgboard->PyArray_DATA;
}



PyMODINIT_FUNC PyInit_BoardPy(void) {
  import_array();
  PyObject *m;
  boardPy_Type.tp_new=PyType_GenericNew;
  if (PyType_Ready(&boardPy_Type) < 0)
    return NULL;

  m = PyModule_Create(&boardPymodule);
  if (m == NULL)
      return NULL;

  Py_INCREF(&boardPy_Type);
  PyModule_AddObject(m, "BoardPy", (PyObject *)&boardPy_Type);
  return m;

}
