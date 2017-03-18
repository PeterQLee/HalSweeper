#include "board_module.h"
//debug functions
#define PY_ARRAY_UNIQUE_SYMBOL

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
  //if (self->imgboard) //temp
  //  Py_DECREF(self->imgboard);
  if (self->mineboard) {
    Py_DECREF(self->mineboard);
  }
  
  Py_TYPE(self)->tp_free((PyObject*)self);
  printf("DEALLOC\n");
}
static int boardPy_init(boardPy *self, PyObject *args, PyObject *kwds){
  self->imgboard=NULL;
  self->mineboard=NULL;
  self->clickboard=NULL;
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

static PyObject * score(boardPy *self, PyObject *args){
  int clicked,mine,num,i,j,sc=0;
  
  for (i=0;i<self->xsize;i++){
    for (j=0;j<self->ysize;j++) {
      clicked=self->clicks->board[COORD(i,self->ysize,j)];
      mine=self->mines->board[COORD(i,self->ysize,j)] & 0x10;
      num=self->mines->board[COORD(i,self->ysize,j)] & 0xF;
      if (clicked && num && !mine) {
	sc++;
      }
    }
  }
  return Py_BuildValue("i",sc);
}

static PyObject * click (boardPy *self, PyObject *args){
  int x,y;

  if (!PyArg_ParseTuple(args,"ii",&x,&y)) return NULL;
  //todo:  make sure x and y are in range
  int bz=self->clicks->board[x*(self->ysize)+y];
  int m=self->mines->board[x*(self->ysize)+y];
  if (bz>0) return Py_BuildValue("i",1); //already clicked blank square
  if (bz==0 && (m & 0x10)){
    //game loss, do something
    permeate_click(self,x,y);
    build_image(self);
    return Py_BuildValue("i",2);

  }
  else{
    permeate_click(self,x,y);

  }

  build_image(self);
  return Py_BuildValue("i",0);
  
  
}
static int permeate_click(boardPy *b, int x, int y) {
  int lx,ly,index,_m;
  long o,px,py;
  
  queue_t * Q=create_queue(MAX_QUEUE_SIZE);
  push_Q(Q,(long)COORD(x,b->ysize,y));
  while (Q->cursize>0){
    o=pop_Q(Q,&_m);
    if (_m) return 1;
    px=XCOORD(o,b->ysize);
    py=YCOORD(o,b->ysize);

    if (b->mines->board[o]==0 && b->clicks->board[o]==0){
      for (lx=-1;lx<=1;lx++){
  	for (ly=-1;ly<=1;ly++){
  	  if (!(lx&ly) && px+lx<(b->xsize) && px+lx>=0 && py+ly<(b->ysize) && py+ly>=0) {
  	    push_Q(Q,(long)COORD(px+lx,b->ysize,py+ly));
  	  }
  	}
      }
     
    }
    b->clicks->board[o]=1;    
  }
  clear_queue(Q);
  return 0;
}
static PyObject * remake_wrapper(boardPy *self, PyObject *args){
  if (self->clickboard)
    Py_DECREF(self->clickboard);
  
  destroy_board(self->mines);
  destroy_board(self->clicks);
  self->mines=malloc(sizeof(board_t));
  self->mines->board=NULL;
  self->clicks=malloc(sizeof(board_t));
  self->clicks->board=NULL;
  remake(self,args);
  return Py_BuildValue("");

    
}
static void remake (boardPy *self, PyObject *args){
  //TODO: check errors
  int i;
  npy_intp x=0,y=0;
  double keep_prob=0.15;
  
  if (!PyArg_ParseTuple(args,"ii",&x,&y)) return;
  npy_intp dims[2]={x*SQUARE_SIZE,y*SQUARE_SIZE};
  npy_intp di[2]={x,y};
  npy_intp *dat=calloc(x*y*SQUARE_SIZE*SQUARE_SIZE,sizeof(npy_intp)); //possible mem leak??
  npy_intp *minedat=calloc(x*y,sizeof(npy_intp));
  PyObject *min_npy=PyArray_SimpleNewFromData(2,di,NPY_INT,minedat);
  PyObject * img_npy=PyArray_SimpleNewFromData(2,dims,NPY_INT,dat);
  
  if (self->imgboard) {
    Py_DECREF(self->imgboard);
    free (self->imgdat);
  }
  if (self->mineboard) {
    Py_DECREF(self->mineboard);
    free (self->minedat);
  }
  self->minedat=minedat;
  self->mineboard=(PyArrayObject*) min_npy;
  Py_INCREF(self->mineboard);
  
  self->imgdat=dat;
  self->imgboard=(PyArrayObject *)img_npy;
  Py_INCREF(self->imgboard);
  genboard(x,y,keep_prob,self->mines,self->clicks);  //make underlying primitive board

  self->clickboard=PyArray_SimpleNewFromData(2,di,NPY_UINT8,self->clicks->board);
  Py_INCREF(self->clickboard);
  
  self->ysize=y;
  self->xsize=x;
  build_image(self);
}



static void build_image(boardPy *self) {//builds pixel image
  int i,j,k,f=0,n;
  int clicked,num,mine;
  //self->imgboard=PyArray_ContiguousFromAny(self->imgboard,NPY_INT,2,2); //unoptimal
  char * map=self->imgboard->data;//PyArray_BYTES(self->imgboard);
  npy_int * minedata=self->mineboard->data;
  if (self->imgboard->strides[0]!=self->ysize*SQUARE_SIZE*COLOUR_OFFSET)
    printf("AHEAEFAE\n");
  char * img_buffer[self->ysize];
  for (i=0;i<self->xsize;i++) {
    
    for (j=0;j<self->ysize;j++) {
      clicked=self->clicks->board[COORD(i,self->ysize,j)];
      mine=self->mines->board[COORD(i,self->ysize,j)] & 0x10;
      num=self->mines->board[COORD(i,self->ysize,j)] & 0xF;

      switch (clicked) {
      case 1://clicked
	switch(mine){
	case 16://visible mine
	  img_buffer[j]=IMAGE(mine);
	  break;
	default://no mine
	  switch (num){
	  case 0:
	    img_buffer[j]=IMAGE(empty);
	    minedata[COORD(i,self->ysize,j)]=0;
	    break;
	  case 1:
	    img_buffer[j]=IMAGE(1);
	    minedata[COORD(i,self->ysize,j)]=1;
	    break;
	  case 2:
	    img_buffer[j]=IMAGE(2);
	    minedata[COORD(i,self->ysize,j)]=2;
	    break;
	  case 3:
	    img_buffer[j]=IMAGE(3);
	    minedata[COORD(i,self->ysize,j)]=3;
	    break;
	  case 4:
	    img_buffer[j]=IMAGE(4);
	    minedata[COORD(i,self->ysize,j)]=4;
	    break;
	  case 5:
	    img_buffer[j]=IMAGE(5);
	    minedata[COORD(i,self->ysize,j)]=5;
	    break;
	  case 6:
	    img_buffer[j]=IMAGE(6);
	    minedata[COORD(i,self->ysize,j)]=6;
	    break;
	  case 7:
	    img_buffer[j]=IMAGE(7);
	    minedata[COORD(i,self->ysize,j)]=7;
	    break;
	  case 8:
	    img_buffer[j]=IMAGE(8);
	    minedata[COORD(i,self->ysize,j)]=8;
	    break;
	  default:
	    printf("ERROR, image buffer\n");
	    break;
	  }
	  break;
	break;
	}
	break;
      default://unclicked
	img_buffer[j]=IMAGE(unclick);
	minedata[COORD(i,self->ysize,j)]=-1;
	break;
      }
	
    }
    for (k=0;k<SQUARE_SIZE ;k++) {
      for (j=0;j<self->ysize;j++) {
	//printf("%d %d",j,k);
	/* if (i==0 && k==0) { */
	/*   printf("\nj = %d %d\n",j, (j+k)*); */
	/*   for (n=0;n<SQUARE_SIZE*COLOUR_OFFSET;n++) { */
	/*     printf ("%d ",*(img_buffer[j]+n)); */
	/*   } */
	/* } */
	
	/*if (j==0 && i==0) {
	  for (n=0;n<SQUARE_SIZE*COLOUR_OFFSET;n++){
	    printf ("T%d ", *(img_buffer[j]+k*SQUARE_SIZE*COLOUR_OFFSET+n));
	    if (n%4==3) printf(",");
	  }
	  printf("\n");
	  }*/
	memcpy(map+(f++)*SQUARE_SIZE*COLOUR_OFFSET,img_buffer[j]+ k*SQUARE_SIZE*COLOUR_OFFSET,SQUARE_SIZE*COLOUR_OFFSET);
      }
    }
    //printf("f= %d",f);
  }
  
  /* printf("%d\n",self->xsize*SQUARE_SIZE); */
  /* for (i=0;i<self->xsize*SQUARE_SIZE;i++){ */
  /*   for (j=0;j<self->ysize*SQUARE_SIZE*COLOUR_OFFSET;j++){ */
  /*     printf("%d ",map[i*self->ysize*SQUARE_SIZE*COLOUR_OFFSET+j]); */
  /*     if (j%4==3) printf(","); */
  /*   } */
  /*   printf("\n"); */
  /* } */

  /* for (i=0;i<SQUARE_SIZE*self->xsize*COLOUR_OFFSET;i++) { */
  /*   printf("%d ",map[i]);  */
  /*   if (i%4==3) printf(","); */
  /*   } */
}


PyMODINIT_FUNC PyInit_BoardPy(void) {
  import_array();
  srand(1);
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
