#include <stdio.h>

#include "gof.h"

// TODO: Make main run interactively: user presses enter => compute new generation, user enter q => quit;
// TODO: Test out rule implementation by testing the glider (from wikipedia)

int pattern[] =
  // Penta Decathlon
  /*
  {
    5, 5,   5, 6,    5, 7,
    6, 5,            6, 7,
    7, 5,   7, 6,    7, 7,
    8, 5,   8, 6,    8, 7,
    9, 5,   9, 6,    9, 7,
    10, 5,  10, 6,   10, 7,
    11, 5,           11, 7,
    12, 5,  12, 6,   12, 7,
    -1
  };
//*/

  // Glider
///*
  {
  1, 1,
  3, 1,
  2, 2,
  3, 2,
  2, 3,
  -1
};
  //*/

  // Blinking Bar
  /*
{
  3, 1,
  3, 2,
  3, 3,
  -1
  }; //*/

int
main(void)
{
  int i, quit = 0, c;
  int *pp;
  Gof *board = gof_create(40, 13);

  //for (i = 0; i < gof_width(board) && i < gof_height(board); i++) {
  //  gof_set(board, i, i, GOF_LIFE);
  //}
  for (pp = pattern; *pp != -1; pp += 2) {
    int x = pp[0], y = pp[1];
    gof_set(board, x, y, GOF_LIFE);
  }

  while (!quit) {
    gof_print(board);
    printf("step? [Y/n] ");
    c = getchar();
    if (c == 'n' || c == EOF) {
      quit = 1;
    } else {
      gof_step(board);
    }
  }

  gof_destroy(board);
  
  printf("bye.\n");
  return 0;
}
