/*GRUPO 12
fc46411 Pedro Gaspar, fc48359 Ana Godinho, fc49034 Ana Fidalgo
*/
#ifndef _TABLE_H
#define _TABLE_H
#include <stdio.h>
#include "entry.h"
#include "table-private.h"

struct table_t;
/* A definir pelo grupo em table-private.h */

/* Função para criar/inicializar uma nova tabela hash, com n
* linhas(n = módulo da função hash)
*/
struct table_t *table_create(int n){

  if (n < 1)
    return NULL;

  struct table_t *t = (struct table_t*) malloc(sizeof(struct table_t));

  if (t == NULL)
    return NULL;

  t -> maxsize = n;
  t -> size = 0;
  t -> nextEmpty = n-1;
  t -> nColls = 0;

  t -> array = (struct entry_t *) calloc(n,sizeof(struct entry_t));

  if (t -> array == NULL ){
    free(t);
    return NULL;
  }

  for (int i=0 ; i < n; i++){
    entry_initialize(&t -> array[i]);
  }

  return t;
}


/* Libertar toda a memória ocupada por uma tabela.
*/
void table_destroy(struct table_t *table){
  if (table != NULL && table -> array != NULL){
    for (int i =0; i < table -> maxsize; i++){
	if (table->array[i].key != NULL){
		free(table->array[i].key);
		data_destroy(table->array[i].value);
	}
    }
    free(table->array);
    free(table);
  }
}

int hash (char* key, int maxsize){

  if (key == NULL)
	return -1;

  int soma = 0;
  int palavra = strlen(key);

  if (palavra <= 4)
    for(int i = 0; i < palavra; i++){
      soma+= key[i];
    }
  else
    soma = key[0] + key[1] + key[palavra-1] + key[palavra-2];

  return soma % maxsize;
}

struct entry_t* find(struct table_t *table, char *key){
  if (table == NULL || key == NULL)
    return NULL;
  int index = hash(key, table -> maxsize);
  struct entry_t* curr = &table -> array[index];
  if (curr == NULL)
	return NULL;
  while (curr -> key != NULL){
    if (strcmp(curr -> key, key) == 0){
      return curr;
    }
    if (curr -> next == NULL)
      return NULL;
    curr = curr->next;
  }
  return NULL;
}

/* \
Função para adicionar um par chave-valor na tabela.
* Os dados de entrada desta função deverão ser copiados.
* Devolve 0 (ok) ou -1 (out of memory, outros erros)
*/
int table_put(struct table_t *table, char *key, struct data_t *value){
 if (table == NULL || key == NULL || value == NULL || table -> maxsize == table -> size) return -1;
 struct entry_t *e = find(table,key);
 if (e != NULL) return -1;
 struct entry_t *a;
 int index = hash(key,table -> maxsize);
 //sem colisao
 if (table -> array[index].key == NULL){
   table -> array[index].key = strdup(key);
   table -> array[index].value = data_dup(value);
   if (index == table -> nextEmpty) calcNextEmpty(table);
 }else{
 //com colisao
    a = &table -> array[index];
    while(a -> next != NULL){
     a = a -> next;
    }
    a -> next = &table -> array[table -> nextEmpty];
    a = a -> next;
    a -> key = strdup(key);
    a -> value = data_dup(value);
    calcNextEmpty(table);
    table->nColls++;
 }
 table -> size++;
 return 0;
}

void calcNextEmpty (struct table_t *table){
 if (table == NULL) return;
 int i = table -> nextEmpty;
 while (i >= 0 && table -> array[i].key != NULL){
  i--;
 }
 table -> nextEmpty = i;
}

/* Função para substituir na tabela, o valor associado à chave key.
* Os dados de entrada desta função deverão ser copiados.
* Devolve 0 (OK) ou -1 (out of memory, outros erros)
*/
int table_update(struct table_t *table, char *key, struct data_t *value){
	if (table == NULL || key == NULL || value == NULL)
    	return -1;
	struct entry_t* e = find(table,key);
	if (e == NULL )
		return -1;
	if (e -> value != NULL)
		data_destroy(e -> value);
	return (e->value = data_dup(value))== NULL? -1 :0;
}

/* Função para obter da tabela o valor associado à chave key.
* A função deve devolver uma cópia dos dados que terão de
* ser libertados no contexto da função que chamou table_get.
* Devolve NULL em caso de erro.
*/
struct data_t *table_get(struct table_t *table, char *key){
  if (table == NULL || key == NULL)
    return NULL;
  struct entry_t *t = find(table,key);
  if (t == NULL){
    return NULL;
  }
return data_dup(t -> value);
}

/* Devolve o número de elementos na tabela.
*/
int table_size(struct table_t *table){
  if (table == NULL)
    return -1;
  return table -> size;
}

/* Devolve um array de char * com a cópia de todas as keys da
* tabela, e um último elemento a NULL.
*/
char **table_get_keys(struct table_t *table){

  if (table == NULL)
    return NULL;

  int next= 0;

  char** res = (char**) malloc (sizeof(char*) * (table -> size + 1));

  if (res == NULL)
    return NULL;

  for (int i =0; i < table -> maxsize; i++)
	if(table -> array[i].key != NULL){
		res[next] = strdup(table -> array[i].key);
		next++;
	}
  res[table->size] = NULL;
  return res;
}

/* Liberta a memória alocada por table_get_keys().
*/
void table_free_keys(char **keys){
	int i =0;
	if (keys == NULL)
		return;
	while (keys[i] != NULL){
		free(keys[i]);
		i++;
	}
	free(keys);
}

int table_num_colls( struct table_t *table){
	if (table == NULL)
		return -1;
	return table->nColls;
}


#endif
