#include "arvb.h"

/*
status -> indica se o arquivo está consitente
noRaiz -> guada a raiz da árvore
proxRRN -> guarda o proximo RRN livre para ser usado caso um novo nó seja criado
nroChaves -> quantidade de chaves guardadas na arvore
o cabeçalho tem que ter 60 bytes então colocamos os últimos 47 como lixo
*/
struct cabecalho_ab {
    char status;
    int noRaiz;
    int proxRRN;
    int nroChaves;
    char lixo[47];
};

/*
char status - indica a consistência do arquivo de dados,
'0' = inconsistente/ '1' = consistente
ao abrir o arquivo para escrita o status deve ser '0' e ao finalizar '1'

long int topo - armazena o byte offset de um registro
logicamente removido, ou -1 caso não tenha - para esse trabalho
essa variável sempre tem valor -1

long int proxByteOffset - armazena o endereço do
próximo byte offset disponível, inicializado com valor 0

int nroRegArq - armazena o número de registros não removidos
presentes no arquivo, inicializado com valor 0

int nroRegRem - armazena o número de registros logicamente
marcados como removido, inicializado com valor 0
*/
struct cabecalho_ {
    char status;
    long int topo;
    long int proxByteOffset;
    int nroRegArq;
    int nroRegRem;
};

/*
removido - indica se o registro está logicamente
removido
'0'= registro não está removido/ '1' = removido

tamanhoRegistro - guarda o número de bytes do registro

Prox - guarda o  byte offset do próximo
registro marcado como removido

id - código identificador do jogador

idade - idade do jogador

tamNomeJog - representa o tamanho
da string que será guardada como o nome do jogador

nomeJogador - ponteiro para a primeira posição do nome do jogador

tamNacionalidade - representa o tamanho
da string que será guardada como a nacionalidade do jogador

nacionalidade - ponteiro para a
primeira posição da onde está a armazenada a nacionalidade do jogador

tamNomeClube -  representa o tamanho
da string que será guardada como o clube do jogador

nomeClube - ponteiro para a
primeira posição de onde está armazenado o nome do clube
*/
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

// função para funcionalidade [7] - Cria o arquivo de índice árvore-B
int CREATE_INDEX_ARVB(char *nomeArquivoBinario, char *nomeArquivoIndice) {
    FILE *fbin = fopen(nomeArquivoBinario, "rb"); // abrir arquivo para leitura do binario com os jogadores
    if (nomeArquivoBinario == NULL || (fbin == NULL)) {
        printf("Falha no processamento do arquivo.\n");
        return 0;
    }
    
    FILE *findex = fopen(nomeArquivoIndice, "wb+"); // abrir/criar arquivo para escrita em binário
    if (nomeArquivoIndice == NULL || (findex == NULL)) {
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

    //aloca e inicializa o cabeçalho com todos os campos "Nulos"
    CABECALHO_AB *cab_ab = inicializar_cabecalho_ab();
    escrever_cabecalho_ab(findex, cab_ab); // escreve o cabeçalho e já pula os 60 primeiros bytes 

    // ponteiro que vai armazenar dados
    JOGADOR *atual;
    long int byteoffset = 25;

    // caso exista registros adicionados, começar a ler sequencialmente
    // caso o registro lido não esteja logicamente removido, add na lista
    if (cab->nroRegArq) {
        while ((atual = ler_linha_bin(fbin))) {                                            // lendo o registro atual (em dados)
            if (atual->removido == '0') ab_inserir(findex, atual->id, byteoffset, cab_ab); // nao removido => inserir na lista encadeada
            byteoffset += atual->tamanhoRegistro;                                          // calculamos o byteoffset para o proximo
            desalocar_struct(&atual);
        }
    } else {
        printf("Registro inexistente.\n\n");
    }

    //pós todas operações, o arquivo está consistente
    cab_ab->status = '1';
    escrever_cabecalho_ab(findex, cab_ab);

    // desalocar todas as structs necessárias
    free(cab);
    free(cab_ab);
    fclose(fbin);
    fclose(findex);

    return 1;
}
