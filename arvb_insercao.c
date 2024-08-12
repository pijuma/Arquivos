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

/*
funcionalidade [10] - Inserir dados em uma tabela com árvore-B
*/
void INSERT_INTO_ARVB(char *nomeArquivoBinario, char *nomeArquivoIndice) {
    FILE *findex = fopen(nomeArquivoIndice, "rb+"); // abrir/criar arquivo para leitura e escrita em binário
    if (nomeArquivoIndice == NULL || (findex == NULL)) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *fbin; // abrir/criar arquivo para leitura em binário
    if (nomeArquivoBinario == NULL || !(fbin = fopen(nomeArquivoBinario, "rb+"))) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // aloca memoria para cabecalho, e lê os primeiros 25 bytes
    CABECALHO *cab = (CABECALHO *)malloc(sizeof(CABECALHO));
    ler_cabecalho(fbin, cab);
    if (cab->status != '1') {
        free(cab);
        fclose(fbin);
        fclose(findex);
        printf("Falha no processamento do arquivo.\n");
        return;
    }
    
    //aloca e lê os 60 primeiros bytes do cabeçalho da ÁrvoreB
    CABECALHO_AB *cab_ab = ler_cabecalho_ab(findex);
    if (cab_ab->status != '1') { //se está incosistente -> desaloca todos para evitar memory leak
        //e retorna que teve falha pra processar
        free(cab);
        free(cab_ab);
        fclose(fbin);
        fclose(findex);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // ler a quantidade de insercoes
    int n;
    scanf("%d", &n);

    // inserir os n registros
    // fluxo:
    // 1. ler o registro
    // 2. inserir best fit no arquivo de dados
    // 3. inserir no arquivo de índices de árvore-B
    // 4. desalocar o registro lido
    // 5. aumentar o número de registros adicionados
    while (n--) {
        JOGADOR *novo = ler_jogador();
        long int byteoffset = inserir_best_fit(fbin, novo, cab);
        ab_inserir(findex, novo->id, byteoffset, cab_ab);
        desalocar_struct(&novo);
        cab->nroRegArq++;
    }

    // reescrever cabeçalho com os dados atualizados
    cab->status = '1';
    fseek(fbin, 0, SEEK_SET);
    escrever_cabecalho(fbin, cab);

    // reescrever o arquivo de índice com os dados atualizados
    cab_ab->status = '1';
    escrever_cabecalho_ab(findex, cab_ab);

    // desalocar todas as structs necessárias
    free(cab);
    free(cab_ab);
    fclose(fbin);
    fclose(findex);

    binarioNaTela(nomeArquivoBinario);
    binarioNaTela(nomeArquivoIndice);
}
