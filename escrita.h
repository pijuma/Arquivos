#ifndef ESCRITA_H
#define ESCRITA_H
#include <stdio.h>
#include <stdlib.h>

typedef struct cabecalho_ CABECALHO;
typedef struct jogador_ JOGADOR;

void CREATE_TABLE(char *nomeCsv, char *nomeArquivoBinario);
void pula_linha(FILE *f);
int getInt(char *s, int tam);
int preencherInt(FILE *f, int *campo);
int preencherString(FILE *f, char **campo);
JOGADOR *ler_linha(FILE *f);
void escrever_binario(FILE *f, JOGADOR *atual);
void escrever_cabecalho(FILE *f, CABECALHO *cab);
void inicializar_cab(CABECALHO *cab);
void desalocar_struct(JOGADOR **atual);

#endif