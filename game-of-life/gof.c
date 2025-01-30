#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

#include "gof.h"

#include "util.h"

struct Gof {
  int width, height;
  GofState *cells;
};

Gof *
gof_create(int width, int height)
{
  Gof *game;
  int i;
  if (width < 0) error("gof_create: width must be positive");
  if (height < 0) error("gof_create: height must be positive");
  if (!(game = calloc(1, sizeof(Gof))))
    error("gof_create: couldn't allocate memory for Gof");
  game->width = width;
  game->height = height;
  if (!(game->cells = calloc(width * height, sizeof(GofState))))
    error("gof_create: couldn't allocate memory for cells");
  for (i = 0; i < width * height; i++)
    game->cells[i] = GOF_DEAD;
  return game;
}

void
gof_destroy(Gof *game)
{
  if (!game) error("gof_destroy: game was NULL");
  if (!game->cells) error("gof_destroy: game->cells was NULL");
  free(game->cells);
  free(game);
}

int
gof_width(Gof *game)
{
  if (!game) error("gof_width: game was NULL");
  return game->width;
}

int
gof_height(Gof *game)
{
  if (!game) error("gof_height: game was NULL");
  return game->height;
}

static int
board_index(Gof *game, int x, int y)
{
  return game->height * x + y;
}

void
gof_set(Gof *game, int x, int y, GofState state)
{
  if (!game) error("gof_set: game was NULL");
  if (!game->cells) error("gof_set: game->cells was NULL");
  if (game->width <= x || x < 0) error("gof_set: x out of range");
  if (game->height <= y || y < 0) error("gof_set: y out of range");
  game->cells[board_index(game, x, y)] = state;
}

GofState
gof_get(Gof *game, int x, int y)
{
  if (!game) error("gof_get: game was NULL");
  if (!game->cells) error("gof_get: game->cells was NULL");
  if (game->width <= x || x < 0) error("gof_get: x out of range");
  if (game->height <= y || y < 0) error("gof_get: y out of range");
  return game->cells[board_index(game, x, y)];
}

static int
is_alive(Gof *game, int x, int y)
{
  int i, width, height;
  GofState neighbour, cell;
  int life_neighbours = 0;
  int neighbours[8][2] = {
    {-1, -1}, {0, -1}, {1, -1},
    {-1, 0},          {1, 0},
    {-1, 1}, {0, 1}, {1, 1}};
  width = gof_width(game), height = gof_height(game);
  for (i = 0; i < 8; i++) {
    neighbour = gof_get(game,
			(width + x + neighbours[i][0]) % width,
			(height + y + neighbours[i][1]) % height);
    if (neighbour == GOF_LIFE) life_neighbours += 1;
  }
  cell = gof_get(game, x, y);
  if (cell == GOF_LIFE && (life_neighbours == 2 || life_neighbours == 3))
    cell = GOF_LIFE;
  else if (cell == GOF_LIFE)
    cell = GOF_DEAD;
  else if (cell == GOF_DEAD && life_neighbours == 3)
    cell = GOF_LIFE;
  else
    cell = GOF_DEAD;
  return (GOF_LIFE == cell);
}

void
gof_step(Gof *game)
{
  Gof *newgame;
  GofState *cells;
  int x, y;
  if (!game) error("gof_step: game is NULL");
  newgame = gof_create(gof_width(game), gof_height(game));

  for (x = 0; x < gof_width(game); x++) {
    for (y = 0; y < gof_height(game); y++) {
      gof_set(newgame, x, y, is_alive(game, x, y) ? GOF_LIFE : GOF_DEAD );
    }
  }

  // Swap cells
  cells = newgame->cells;
  newgame->cells = game->cells;
  game->cells = cells;
  gof_destroy(newgame);
}

void
gof_print(Gof *game)
{
  int x, y;
  GofState state;
  printf("+");
  for (x = 0; x < 2*gof_width(game)+1; x++)
    printf("-");
  printf("+\n");
  for (y = 0; y < gof_height(game); y++) {
    printf("|");
    for (x = 0; x < gof_width(game); x++) {
      state = gof_get(game, x, y);
      printf(" %c", (state == GOF_LIFE) ? '@' : '.');
    }
    printf(" |\n");
  }
  printf("+");
  for (x = 0; x < 2*gof_width(game)+1; x++)
    printf("-");
  printf("+\n");
}

