#ifndef LISTA_H
#define LISTA_H

#include <stdbool.h>

typedef struct lista_ LISTA;
typedef struct item_ ITEM;
typedef struct no_ NO;

LISTA *lista_criar(void);
int lista_inserir_ini(LISTA *l, ITEM *x);
int lista_inserir_fim(LISTA *l, ITEM *x);
int lista_inserir_ord(LISTA *l, ITEM *x);
ITEM *lista_buscar(LISTA *l, int chave);
ITEM *lista_remover(LISTA *l, int chave);
int lista_apagar(LISTA **l);
int lista_apagar(LISTA **l);
int lista_tamanho(LISTA *l);
int lista_vazia(LISTA *l);
void lista_imprimir(LISTA *l);

ITEM *item_criar(int id, long int byteoffset);
bool item_apagar(ITEM **item);
int item_get_id(ITEM *item);
long int item_get_byteoffset(ITEM *item);
bool item_set_chave(ITEM *item, int id, long int byteoffset);

#endif