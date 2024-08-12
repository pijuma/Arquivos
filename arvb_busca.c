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
função que chama a busca na árvoreb caso
a busca seja pelo indice 
[8]: Busca apenas o ID utilizando o índice árvore-B
*/
void WHERE_ARVB_ID(char *nomeArquivoBinario, char *nomeArquivoIndice) {
    FILE *findex = fopen(nomeArquivoIndice, "rb"); // abrir/criar arquivo para escrita em binário

    if (nomeArquivoIndice == NULL || (findex == NULL)) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *fbin; // abrir/criar arquivo para leitura em binário

    if (nomeArquivoBinario == NULL || !(fbin = fopen(nomeArquivoBinario, "rb"))) {
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

    // aloca cabeçalho arvoreb e lê os primeiros 60 bytes 
    CABECALHO_AB *cab_ab = ler_cabecalho_ab(findex);
    if (cab_ab->status != '1') {
        free(cab);
        free(cab_ab);
        fclose(fbin);
        fclose(findex);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    int q; // quantidade de buscas
    scanf("%d", &q);
    int id;
    for (int i = 1; i <= q; i++) {
        char op[20];
        scanf("%s", op);
        if (!strcmp(op, "id")) {
            scanf("%d", &id);
        }

        printf("BUSCA %d\n\n", i);
        long int byteoffset = ab_buscar(findex, id, cab_ab);
        // caso não tenha encontrado, ou caso não existam registros no arquivo binário, printa o abaixo:
        if (byteoffset != -1) {
            fseek(fbin, byteoffset, SEEK_SET);
            JOGADOR *resultado = ler_linha_bin(fbin);
            printar_registro(resultado);
            desalocar_struct(&resultado);
        } else
            printf("Registro inexistente.\n\n");

        // se não for a última busca a fazer, voltar o ponteiro para o início do arquivo
        if (i != q)
            fseek(fbin, 25, SEEK_SET);
    }

    // desalocar todas as structs necessárias
    free(cab);
    free(cab_ab);
    fclose(fbin);
    fclose(findex);
}

// [9]: Busca utilizando o índice árvore-B
void WHERE_ARVB(char *nomeArquivoBinario, char *nomeArquivoIndice) {
    FILE *findex = fopen(nomeArquivoIndice, "rb"); // abrir/criar arquivo para escrita em binário

    if (nomeArquivoIndice == NULL || (findex == NULL)) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    FILE *fbin; // abrir/criar arquivo para leitura em binário

    if (nomeArquivoBinario == NULL || !(fbin = fopen(nomeArquivoBinario, "rb"))) {
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

    //alocar memoria para cabeçalho da arvoreb e ler os 60 primeiros bytes 
    CABECALHO_AB *cab_ab = ler_cabecalho_ab(findex);
    if (cab_ab->status != '1') {
        free(cab);
        free(cab_ab);
        fclose(fbin);
        fclose(findex);
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    int q; // quantidade de buscas
    scanf("%d", &q);
    JOGADOR *busca = (JOGADOR *)malloc(sizeof(JOGADOR));

    for (int i = 1; i <= q; i++) {

        int n;
        scanf("%d ", &n); // iremos buscar por n campos
        inicializar_null(busca);

        for (int j = 1; j <= n; j++) {
            ler_criterio(busca);
        }

        JOGADOR *atual; // ponteiro que vai armazenar dados do registro atual
        printf("Busca %d\n\n", i);

        // como o cabecalho foi lido o ponteiro está no 1o registro já
        // caso existam registros, começar a ler.
        if (cab->nroRegArq) {
            // se um dos criterios de busca é o ID, só iremos remover um registro
            if (busca->id != -1) {
                long int byteoffset = ab_buscar(findex, busca->id, cab_ab);
                // caso não tenha encontrado, ou caso não existam registros no arquivo binário, printa o abaixo:
                if (byteoffset != -1) {
                    fseek(fbin, byteoffset, SEEK_SET);
                    atual = (JOGADOR *)malloc(sizeof(JOGADOR));
                    ler_struct(fbin, atual);
                    printar_registro(atual);
                    desalocar_struct(&atual);
                } else {
                    printf("Registro inexistente.\n\n");
                }
            } else {
                int qtdRes = 0;
                while ((atual = ler_linha_bin(fbin))) {
                    // se o atual não foi removido ainda, checamos se deve ser removido
                    // a função comparar_remover retorna 1 caso o registro "atual" seja removido
                    if (atual->removido != '1')
                        if (comparar(busca, atual, &qtdRes) && busca->id != -1) {
                            desalocar_struct(&atual);
                            break;
                        }
                    desalocar_struct(&atual);
                }
                // caso não tenha encontrado, ou caso não existam registros no arquivo binário, printa o abaixo:
                if (qtdRes == 0) printf("Registro inexistente.\n\n");
            }

            desalocar_necessario(busca);

            // se não for a última busca a fazer, voltar o ponteiro para o início do arquivo
            if (i != q)
                fseek(fbin, 25, SEEK_SET);
        }
    }

    // desalocar todas as structs necessárias
    free(busca);
    free(cab);
    free(cab_ab);
    fclose(fbin);
    fclose(findex);
}
