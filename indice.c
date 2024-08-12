#include "indice.h"

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

struct jogador_ {
    char removido;
    int tamanhoRegistro;
    long int Prox;
    int id;
    int idade;
    int tamNomeJog;
    char *nomeJogador;
    int tamNacionalidade;
    char *nacionalidade;
    int tamNomeClube;
    char *nomeClube;
};

struct cabecalho_ {
    char status;
    long int topo;
    long int proxByteOffset;
    int nroRegArq;
    int nroRegRem;
};

/*
Nessa função iremos percorrer a Lista encadeada criada para 
passar as informações da lista (já ordenada) para o arquivo de indices
*/
void escrever_arquivo_indice(FILE *f, LISTA *reg, char status) {
    //fseek(f, 1, SEEK_SET); -> nao precisa,  já fez fseek antes
    // percorrer a lista escrevendo todos os dados
    NO *aux = reg->ini;
    while (aux != NULL) {
        fwrite(&(aux->x->id), sizeof(int), 1, f);
        fwrite(&(aux->x->byteoffset), sizeof(long int), 1, f);
        aux = aux->prox; //proximo item da lista 
    }
    fseek(f, 0, SEEK_SET); //reescrever o status - marcando como '1' (valido)
    fwrite(&(status), sizeof(char), 1, f);
}

/*
função para inserir um jogador (registro) na lista, caso esse não esteja já apagado
*/
void inserir_lista(LISTA *reg, JOGADOR *atual, long int byteoffset) {
    if (atual->removido == '1') return ; //já foi removido, não queremos adicionar na lista 
    ITEM *item = item_criar(atual->id, byteoffset);
    lista_inserir_ord(reg, item);
}

// função para ler o arquivo de índices e adcionar na lista encadeada
LISTA *pegar_indice(FILE *f) {

    // o ponteiro do arquivo já está com o 1o byte pulado
    LISTA *reg = lista_criar();

    // ler cada linha do arquivo de índices e colocar na lista encadeada ordenada
    ITEM *item;
    while ((item = ler_linha_indice(f))) lista_inserir_fim(reg, item);

    return reg;
}

/*
função para ler o registro de indices e colocá-los em um array
para que possamos fazer busca binária posteriormente 
*/
ITEM **cria_array(FILE *f, int nroRegArq) {
    ITEM **array = (ITEM **)malloc(nroRegArq * sizeof(ITEM *));
    // vetor com os itens ordenados
    int ct = -1;
    ITEM *item;
    while ((item = ler_linha_indice(f))) {
        array[++ct] = item;
    }
    return array;
}

/*
função para desalocar o vetor usado para busca binária 
*/
void desaloca_array(ITEM ***array, int tamArray) {
    while (tamArray--) {
        item_apagar(&((*array)[tamArray]));
    }
    free(*array);
}

/*
tendo o vetor (array) com os valores já ordenados por ID 
podemos fazer busca binária para achar o ITEM de forma rápida. 
Achando esse temos acesso ao byteoffset também. 
Essa função irá auxiliar a remoção de um registro, sabendo o ID dele iremos buscar o byteoffset 
para assim podermos acessar o local e marcar o registro como removido de modo mais rápido
*/
ITEM *busca_indice(ITEM **array, int n, int id) {

    int ini = 0, fim = n - 1, mid;

    while (ini <= fim) {
        mid = (ini + fim) >> 1;
        if ((array[mid])->id == id) { //achamos
            return array[mid];
        } else if ((array[mid])->id > id) //estamos buscando um valor menor
            fim = mid - 1;
        else //estamos buscando um valor maior
            ini = mid + 1;
    }

    return NULL; //nao achamos um registro com id 
}

/*
Função que dado um vetor, retorna uma Lista ordenada encadeada com os registros 
armazenados no vetor de forma ordenada pelo ID
*/
LISTA *passar_vetor_indice(ITEM **array, int tamArray) {
    LISTA *reg = lista_criar();
    // caso o elemento do array seja NULL, então ele foi apagado, não insere na lista
    int i = 0;
    while (i < tamArray) {
        if (array[i]->byteoffset != -1) { //se byteoffset = -1 então o elemento foi apagado (definimos assim)
            ITEM *item = item_criar(array[i]->id, array[i]->byteoffset);
            lista_inserir_fim(reg, item);
        }
        i++;
    }

    return reg;
}

/*
função que imprime uma dada lista
*/
void printar_lista(LISTA *reg) {
    NO *aux = reg->ini;
    while (aux != NULL) {
        printf("id: %d byteoffset: %ld\n", aux->x->id, aux->x->byteoffset);
        aux = aux->prox;
    }
}

/*
função para ler a linha do arquivo de indice
lê o ID e byteoffset 
função semelhante à do arquivo de dados 
*/
ITEM *ler_linha_indice(FILE *f) {
    int id;
    long int byteoffset;

    if (!fread(&id, sizeof(int), 1, f)) return NULL;
    fread(&byteoffset, sizeof(long int), 1, f);

    ITEM *item = item_criar(id, byteoffset);
    return item;
}

/*
função principal para criação do arquivo de indices, nela utilizamos todas as outras funções implementadas
dado dois arquivos um de dados e um de indice a função irá abrir o arquivo de dados e o de indices
para ler os registros no arquivo de dados, colocá-los na lista encadeada ordenada para assim passarmos
para o arquivo de indices escrevendo os dados de forma ordenada pelo ID
*/
int CREATE_INDEX(char *nomeArquivoBinario, char *nomeArquivoIndice) {

    FILE *findex = fopen(nomeArquivoIndice, "wb"); // abrir/criar arquivo para escrita em binário

    if (nomeArquivoIndice == NULL || (findex == NULL)) {
        printf("Falha no processamento do arquivo.\n");
        return 0;
    }

    FILE *fbin = fopen(nomeArquivoBinario, "rb"); // abrir arquivo para leitura do binario com os jogadores

    if (nomeArquivoBinario == NULL || (fbin == NULL)) {
        printf("Falha no processamento do arquivo.\n");
        return 0;
    }

    // aloca memoria para cabecalho, e lê os primeiros 25 bytes
    CABECALHO *cab = (CABECALHO *)malloc(sizeof(CABECALHO));
    ler_cabecalho(fbin, cab);

    if (cab->status != '1') {
        printf("Falha no processamento do arquivo.\n");
        free(cab);
        fclose(fbin);
        fclose(findex);
        return 0;
    }

    char status = '0';
    // escrever o status de cabeçalho como invalido até que a escrita/operação seja concluida
    fwrite(&(status), sizeof(char), 1, findex);

    LISTA *reg = lista_criar();
    long int byteoffset = 25;

    // ponteiro que vai armazenar dados
    JOGADOR *atual;
    // como o cabecalho foi lido o ponteiro está no 1o registro já

    // caso exista registros adicionados, começar a ler sequencialmente
    // caso o registro lido não esteja logicamente removido, add na lista
    if (cab->nroRegArq) {
        while ((atual = ler_linha_bin(fbin))) {                                  // lendo o registro atual (em dados)
            if (atual->removido == '0') inserir_lista(reg, atual, byteoffset); // nao removido => inserir na lista encadeada
            byteoffset += atual->tamanhoRegistro;                              // calculamos o byteoffset para o proximo
            desalocar_struct(&atual);
        }
    } else {
        printf("Registro inexistente.\n\n");
    }

    status = '1'; // ao final, o arquivo de indice terá status valido
    escrever_arquivo_indice(findex, reg, status);
    lista_apagar(&reg);

    free(cab);
    fclose(fbin);
    fclose(findex);

    return 1;
}