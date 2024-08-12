#ifndef BUSCA_H
#define BUSCA_H
#include "escrita.h"
#include "leitura.h"
#include "funcoes_fornecidas.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void WHERE(char *nomeArquivoBinario);
void inicializar_null(JOGADOR *busca);
void desalocar_necessario(JOGADOR *busca);
int comparar(JOGADOR *busca, JOGADOR *atual, int *qtdRes);
void ler_criterio(JOGADOR *busca);

#endif