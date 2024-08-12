/*
lista duplamente encadeada ordenada (implementada na disciplina de ED)
*/

#include "lista.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/*
cada nó irá guardar o próprio item 
e um ponteiro para o nó anterior e o próximo (duplamente encadeada)
*/
struct no_ {
    ITEM *x;
    struct no_ *prox, *ant;
};

struct lista_ {
    NO *ini, *fim;
    int tam;
};

struct item_ {
    int id;
    long int byteoffset;
};

/*
inicializando os devidos valores e alocando a lista 
*/

LISTA *lista_criar() {

    LISTA *l = (LISTA *)malloc(sizeof(LISTA));

    if (l != NULL) {
        l->ini = l->fim = NULL;
        l->tam = 0;
    }

    return l;
}

/*
nessa função queremos inserir o item x na lista, criando um novo nó para ele 
e mantendo a lista ordenada 
para isso buscamos onde o valor deve ser inserido 
quando encontrada a posição, fazemos o anterior apontar para o novo nó e com que 
o próximo do atual seja o próximo do anterior, assim ele será inserido no meio dos dois nós 
*/

int lista_inserir_ord(LISTA *l, ITEM *x) {

    if (l == NULL) return 0;

    // buscar o ultimo lugar que eh menor que o item novo (onde irei inserir?)

    NO *at = l->ini;
    NO *novo = (NO *)malloc(sizeof(NO));

    novo->x = x;
    novo->ant = novo->prox = NULL;
    novo->ant = NULL;

    //percorrendo a lista enquanto o valor atual é <= ao meu 
    // quando pela 1a vez for > é ali que devo inserir 
    while (at != NULL && at->prox != NULL && item_get_id((at->prox)->x) <= item_get_id(x))
        at = at->prox;

    // at guarda o ultimo <= a mim
    
    //sou o começo e o fim da lista 
    if (at == NULL) { // lista vazia
        l->ini = l->fim = novo;
        l->tam++;
        return 1;
    }
    
    //sou o novo inicio da lista
    if (item_get_id(at->x) > item_get_id(x)) { // to na 1a pos
        novo->prox = l->ini;
        l->ini->ant = novo;
        l->ini = novo;
    }

    else if (at->prox == NULL) { // sou o ultimo ent meu prox n existe -> sou o novo fim 
        at->prox = novo;
        novo->ant = at;
        l->fim = novo;
    }

    else { // estou entre dois nós 
        novo->ant = at;
        novo->prox = at->prox;
        at->prox = novo;
        (novo->prox)->ant = novo;
    }

    l->tam++;

    return 1;
}


/*
Função para apenas inserir no final da lista.
Útil para quando se sabe que as entradas já estão ordenadas, e não 
precisa percorrer por toda a lista para inserir no final.
*/
int lista_inserir_fim(LISTA *l, ITEM *x) {

    if (l == NULL) return 0;

    NO *novo = (NO *)malloc(sizeof(NO));

    novo->x = x;
    novo->ant = novo->prox = NULL;

    if (l->ini == NULL) l->ini = novo;

    novo->ant = l->fim;
    if (l->fim != NULL) l->fim->prox = novo;
    l->fim = novo;
    l->tam++;

    return 1;
}

/*
função que retorna um ITEM que contenha "chave" caso exista 
*/

ITEM *buscar(NO *at, int chave) {
    if (at == NULL) return NULL;
    if (item_get_id(at->x) == chave) return at->x;
    return buscar(at->prox, chave);
}

// apenas chama a função "buscar"
ITEM *lista_buscar(LISTA *l, int chave) {
    if (l == NULL || lista_vazia(l)) return NULL;
    return buscar(l->ini, chave);
}

/*
queremos remover o nó que contem "chave"
*/
ITEM *lista_remover(LISTA *l, int chave) { 

    if (l == NULL || lista_buscar(l, chave) == NULL) return NULL;

    NO *at = l->ini;

    //devemos achar o nó primeiramente 
    while (at != NULL && item_get_id(at->x) != chave)
        at = at->prox;
    
    // nao achamos -> nao iremos remover 
    if (at == NULL) return NULL;

    //se ele possui um nó anterior a ele queremos fazer esse nó anterior 
    //apontar para o proximo dele, assim retirando o nó a ser removido do meio desses dois nós
    // ant rem prox -> ant prox 
    if (at->ant != NULL)
        (at->ant)->prox = (at)->prox;
    else
        l->ini = at->prox; // se nao possui um anterior = começo da lista -> preciso mudar o inicio 
    //colocando que é o proximo do nó que estou removendo 

    //é o ultimo nó
    if (at->prox == NULL)
        l->fim = at->ant;
    else // se nao é o ultimo nó preciso dizer que o anterior do meu proximo vai ser o anterior do nó a ser removido
        (at->prox)->ant = at->ant;

    ITEM *itemm = at->x;

    //desalocar memória para evitar memory leak 
    at->ant = NULL;
    at->prox = NULL;
    free(at);
    at = NULL;
    l->tam--;

    return itemm;
}

// desalocando a lista (aproveitando a função remover)
int lista_apagar(LISTA **l) {
    if (*l == NULL) return 0;
    while ((*l)->ini != NULL) {
        NO *aux = (*l)->ini->prox;
        item_apagar(&((*l)->ini->x));
        free((*l)->ini);
        (*l)->ini = aux;
    }
    free(*l);
    *l = NULL;
    return 1;
}

//retorna o tamamnho da lista
int lista_tamanho(LISTA *l) { return l->tam; }

//checa se a lista está vazia
int lista_vazia(LISTA *l) { return (l->tam == 0); }

// função para imprimir a lista (utilizada para debugar o código caso precise)
void lista_imprimir(LISTA *l) {

    if (l == NULL || lista_vazia(l)) return;

    NO *at = l->ini;

    while (at != NULL) {
        printf("%d ", item_get_id(at->x));
        at = at->prox;
    }
    printf("\n");

    return;
}

//criar um novo item para então colocarmos no nó 
ITEM *item_criar(int id, long int byteoffset) {
    ITEM *item;

    item = (ITEM *)malloc(sizeof(ITEM));

    if (item != NULL) {
        item->id = id;
        item->byteoffset = byteoffset;
        return (item);
    }
    return (NULL);
}

// desalocar o item (evitar memory leak)
bool item_apagar(ITEM **item) {
    if (*item != NULL) {
        free(*item);
        *item = NULL; /*Boa prática!*/
        return (true);
    }
    return (false);
}

//qual ID do item? (estamos usando TAD)
int item_get_id(ITEM *item) {
    if (item != NULL)
        return (item->id);
    exit(1);
}

//qual byteoffset do item?
long int item_get_byteoffset(ITEM *item) {
    if (item != NULL)
        return (item->byteoffset);
    exit(1);
}

//queremos mudar as informações salvas no item 
bool item_set_chave(ITEM *item, int id, long int byteoffset) {
    if (item != NULL) {
        item->id = id;
        item->byteoffset = byteoffset;
        return (true);
    }
    return (false);
}