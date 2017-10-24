/*GRUPO 12
fc46411 Pedro Gaspar, fc48359 Ana Godinho, fc49034 Ana Fidalgo
*/
#ifndef _TABLE_PRIVATE_H
#define _TABLE_PRIVATE_H

#include "table.h"
#include "entry.h"

struct table_t {
  int size;
  int maxsize;
  int nextEmpty;
  int nColls;
  struct entry_t *array;
};

int hash (char *key, int size);
struct entry_t* find(struct table_t *table, char *key);
void calcNextEmpty (struct table_t *table);
int table_num_colls(struct table_t *table);



#endif
