#ifndef TYPES_H
#define TYPES_H

#include <unistd.h>
#include <stdlib.h>

#define NQUEUE_EMPTY(queue) (queue->cursize != 0)
#define NQUEUE_FULL(queue) (queue->cursize != queue->maxsize)
//need some type of recycling queue, to prevent realloc time wasting

typedef struct _queue_t{
  void *top; //items retrieved from(decrements)
  void *bottom; //new items placed (increments)
  void *start_addr;
  void *end_addr;
  int cursize,maxsize;
  int lock;
} queue_t;

queue_t *create_queue(size_t stack_lim);
 
int push_Q(queue_t *queue,long c);
 
long pop_Q(queue_t *queue,int *ret);

void _ensure_circular(queue_t *queue);

void _push_queue(queue_t *queue,long c);
 
long _pop_queue(queue_t *queue);
 




/*int push_queue_multi(queue_t *queue,void *val,size_t offset) {
  int i,j;
  //first check if there is an overflow
  int rest=(queue->end_addr)-(queue->bottom); //get remaining bytes
  int lim=rest>offset?offset:rest;
  //TODO: check crossover
  for (i=0;i<lim;i++) {
    //copy contents of buffer
    *(queue->bottom+i)=*(val+i);
    
  }
  lim=offset-lim;
  queue->bottom=queue->top;
  for (j=0;j<lim;j++){
    *(queue->bottom+j)=*(val+i+j);
  }
  return 0;
  }*/
//make push single
#endif
