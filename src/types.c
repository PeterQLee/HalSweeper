#include "types.h"
#define LONGINC 8
queue_t *create_queue(size_t stack_lim){
  queue_t *queue=malloc(sizeof(queue_t));
  long *mem=malloc(stack_lim*sizeof(long));
  queue->top=mem;
  queue->bottom=mem;
  queue->start_addr=mem;
  queue->end_addr=mem+((stack_lim-1)*LONGINC);//check this
  queue->cursize=0;
  queue->maxsize=stack_lim;
  return queue;
}
void clear_queue(queue_t *queue) {
  //clean up memory
  free(queue->start_addr);
  free(queue);
}


int push_Q(queue_t *queue,long c) {
  //Access function, ensures queue is circular and pushes.
  _ensure_circular(queue);
  if (NQUEUE_FULL(queue)) {
    _push_queue(queue, c);
    return 0;
  } 
  return 1;
}
long pop_Q(queue_t *queue, int *ret) {
  //Access function, ensures queue is circular and pops
  while (queue->lock) {}
  
  queue->lock=1;
  _ensure_circular(queue);

  if (NQUEUE_EMPTY(queue)) {
    *ret=0;
    return _pop_queue(queue);

  }
  *ret=1;
  queue->lock=0;
  return 0;
  
  
}

void _ensure_circular(queue_t *queue) {
  //make sure pointers are in range
  if (queue->top>queue->end_addr) 
    queue->top=queue->start_addr;
  
  if (queue->bottom>queue->end_addr)
    queue->bottom=queue->start_addr;
  
}

void _push_queue(queue_t *queue,long c) {
  //pushes a single long
  *((long*)queue->bottom)=c;
  queue->bottom+=LONGINC; //increment pointer
  queue->cursize+=LONGINC;
  queue->lock=0;
}

long _pop_queue(queue_t *queue) {
  //Trusts that something is in the queue already
  queue->top+=LONGINC;
  queue->cursize-=LONGINC;
  long d= *((long *)(queue->top-LONGINC));
  queue->lock=0;
  return d;

}
