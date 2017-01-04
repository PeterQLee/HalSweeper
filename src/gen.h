#ifndef GEN_H
#define GEN_H
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
//#include <Python.h>
typedef unsigned char piece;
typedef int colour_t;
typedef struct{
  piece *board;
  int x,y;
}board_t;
typedef struct{
  colour_t ** im;
  int x,y;
}image_t;


colour_t rgb_to_col(unsigned char r, unsigned char g, unsigned char b);

void alloc_board(board_t **t, int n);
void genboard (int x, int y, double keep_prob,board_t *pieceboard, board_t *clickboard);
void destroy_board(board_t *board);
image_t * create_image(board_t *board);


void debug_printboard(board_t *board);
#endif
