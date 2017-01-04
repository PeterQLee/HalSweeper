#include "gen.h"

colour_t rgb_to_col(unsigned char r, unsigned char g, unsigned char b){
  return (colour_t) ((r<<16) & (g<<8) & b);
}

void alloc_board(board_t **t, int n) {
  int i;
  for (i=0;i<n;i++) {
    t[i]=malloc(sizeof(board_t));
  }
}

void genboard(int x, int y,  double keep_prob, board_t * mineboard, board_t * clickboard){
  piece * board=calloc(sizeof(piece),x*y);
  int i,j,k,l;
  
  for (i=0;i<x;i++) {
    for (j=0;j<y;j++) {
      if ((double)rand()/(double)RAND_MAX < keep_prob) {
	board[i*y+j]=0x10;
	//check 9 adjacent
	int kstart= i==0?0:-1;
	int klim= i==x-1?1:2;
	int lstart= j==0?0:-1;
	int llim= j==y-1?1:2;
	for (k=kstart;k<klim;k++){
	  for (l=lstart;l<llim;l++) {
		board[(i+k)*y+j+l]++;
	  }
	}
      }
    }
  }
  mineboard->board=board;
  mineboard->x=x;
  mineboard->y=y;

  clickboard->board=calloc(sizeof(piece),x*y);
  clickboard->x=x;
  clickboard->y=y;
}


void destroy_board(board_t *board){
  free (board->board);
  free (board);
}

image_t * create_image(board_t *board) {
}

void debug_printboard(board_t *board){
  int i,j;
  for (i=0;i<board->x;i++) {
    for (j=0;j<board->y;j++) {
      printf("|%2d|",board->board[i*board->y+j]);
    }
    printf("\n");
  }
  
}

/* int main() { */
/*   board_t *b= genboard(15,15,0,0.25); */
/*   debug_printboard(b); */
/*   destroy_board(b); */
/* } */
