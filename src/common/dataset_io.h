#ifndef DATASET_IO_H
#define DATASET_IO_H

#include "player.h"

void    populate_players(Player *players, int count);
int     write_players_csv(const char *path, int count);
Player *load_players_csv(const char *path, int *out_count);

#endif
