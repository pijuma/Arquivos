#ifndef REMOCAO_H
#define REMOCAO_H
#include "leitura.h"
#include "indice.h"
#include "busca.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int comparar_remover(JOGADOR *busca, JOGADOR *atual, FILE *fbin, CABECALHO *cab, ITEM **array, int tamArray);
void remover_registro(FILE *fbin, long int byteoffset, CABECALHO *cab);
void DELETE(char *nomeArquivoBinario, char *nomeArquivoIndice);

#endif