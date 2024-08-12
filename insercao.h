#ifndef INSERCAO_H
#define INSERCAO_H
#include "indice.h"
#include "leitura.h"
#include "funcoes_fornecidas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

JOGADOR *ler_jogador();
long int inserir_best_fit(FILE *fbin, JOGADOR *novo, CABECALHO *cab);
void INSERT_INTO(char *nomeArquivoBinario, char *nomeArquivoIndice);

#endif