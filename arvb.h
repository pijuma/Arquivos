#ifndef ARVB_H
#define ARVB_H
#include "busca.h"
#include "escrita.h"
#include "funcoes_fornecidas.h"
#include "insercao.h"
#include "leitura.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct cabecalho_ab CABECALHO_AB;
typedef struct no NO_AB;
typedef struct no_aux NO_AUX;

CABECALHO_AB *inicializar_cabecalho_ab();
NO_AUX *criar_aux_null();
NO_AB *inicializar_no();

void escrever_cabecalho_ab(FILE *f, CABECALHO_AB *cab);
CABECALHO_AB *ler_cabecalho_ab(FILE *f);

void ab_criar_raiz(FILE *f, int id, long int byteoffset, CABECALHO_AB *cab);
void ab_escrever_no(FILE *f, int RRN, NO_AB *no);
NO_AB *ab_ler_no(FILE *f, int RRN);

void ab_inserir(FILE *findex, int id, long int byteoffset, CABECALHO_AB *cab);
NO_AUX *ab_inserir_recur(FILE *f, int RRN_atual, int id, long int byteoffset, CABECALHO_AB *cab);
NO_AUX *add_novo_no(FILE *f, int RRN_atual, NO_AB *atual, int id, long int byteoffset, int filho_esq, int filho_dir, CABECALHO_AB *cab);
NO_AB *mudar_pos_chaves(NO_AB *atual, int k1, int k2, int k3, int id, int byteoffset);
void atualizar_no_esq(NO_AB *atual, int k, int id, long int byteoffset, int Pesq, int Pdir);
void INSERT_INTO_ARVB(char *nomeArquivoBinario, char *nomeArquivoIndice);

int CREATE_INDEX_ARVB(char *nomeArquivoBinario, char *nomeArquivoIndice);

long int ab_buscar(FILE *findex, int id, CABECALHO_AB *cab);
long int ab_buscar_recur(FILE *f, int RRN_atual, int id);
void WHERE_ARVB_ID(char *nomeArquivoBinario, char *nomeArquivoIndice);
void WHERE_ARVB(char *nomeArquivoBinario, char *nomeArquivoIndice);


void print_no(NO_AB *no);

#endif