#ifndef LEITURA_H
#define LEITURA_H
#include <stdio.h>
#include <stdlib.h>
#include "escrita.h"

void SELECT(char *nomeArquivoBinario);
void ler_cabecalho(FILE *f, CABECALHO *cab);
JOGADOR *ler_struct(FILE *f, JOGADOR *atual);
void printar_registro(JOGADOR *atual);
JOGADOR *ler_linha_bin(FILE *f);

#endif