#ifndef INDICE_H
#define INDICE_H
#include "funcoes_fornecidas.h"
#include "leitura.h"
#include "lista.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int CREATE_INDEX(char *nomeBin, char *nomeArquivoIndice);
void escrever_arquivo_indice(FILE *f, LISTA *reg, char status);
void inserir_lista(LISTA *reg, JOGADOR *atual, long int byteoffset);
ITEM *ler_linha_indice(FILE *f);
LISTA *pegar_indice(FILE *f);
ITEM **cria_array(FILE *f, int nroRegArq);
void desaloca_array(ITEM ***array, int tamArray);
ITEM *busca_indice(ITEM **array, int id, int n);
LISTA *passar_vetor_indice(ITEM **array, int tamArray);
void printar_lista(LISTA *reg);

#endif
