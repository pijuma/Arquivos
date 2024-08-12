#include "leitura.h"

/*
struct do cabeçalho:

char status - 1byte - indica a consistência do arquivo de dados,
'0' = inconsistente/ '1' = consistente
ao abrir o arquivo para escrita o status deve ser '0' e ao finalizar '1'

long int topo - 8 bytes - armazena o byte offset de um registro
logicamente removido, ou -1 caso não tenha - para esse trabalho
essa variável sempre tem valor -1

long int proxByteOffset - 8 bytes - armazena o endereço do
próximo byte offset disponível, inicializado com valor 0

int nroRegArq - 4 bytes - armazena o número de registros não removidos
presentes no arquivo, inicializado com valor 0

int nroRegRem - 4 bytes - armazena o número de registros logicamente
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
struct jogador:

char removido - 1 byte - indica se o registro está logicamente
removido
'0'= registro não está removido/ '1' = removido

int tamanhoRegistro - 4 bytes - guarda o número de bytes do registro

long int Prox - 8 bytes - guarda o  byte offset do próximo
registro marcado como removido

int id - 4 bytes - código identificador do jogador

int idade - 4 bytes - idade do jogador

int tamNomeJog - 4 bytes - representa o tamanho
da string que será guardada como o nome do jogador

char *nomeJogador - tamNomeJog bytes - ponteiro
para a primeira posição do nome do jogador

int tamNacionalidade - 4 bytes - representa o tamanho
da string que será guardada como a nacionalidade do jogador

char *nacionalidade - tamNacionalidade bytes - ponteiro para a
primeira posição da onde está a armazenada a nacionalidade do jogador

int tamNomeClube - 4 bytes -  representa o tamanho
da string que será guardada como o clube do jogador

char *nomeClube - tamNomeClube bytes - ponteiro para a
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
Função para ler o cabeçalho do arquivo binário
*/
void ler_cabecalho(FILE *f, CABECALHO *cab) {
    fread(&(cab->status), sizeof(char), 1, f);
    fread(&(cab->topo), sizeof(long int), 1, f);
    fread(&(cab->proxByteOffset), sizeof(long int), 1, f);
    fread(&(cab->nroRegArq), sizeof(int), 1, f);
    fread(&(cab->nroRegRem), sizeof(int), 1, f);
}

/*
funcao para ler a struct do arquivo binário
Na hora de ler as strings, é necessário alocar memória para elas
*/
JOGADOR *ler_struct(FILE *f, JOGADOR *atual) {

    // Caso o fread retornar 0, então o arquivo não conseguiu ler nenhum byte,
    // e portanto chegou no final do arquivo. Então retorna um ponteiro nulo
    if (!fread(&(atual->removido), sizeof(char), 1, f)) return NULL;
    fread(&(atual->tamanhoRegistro), sizeof(int), 1, f);
    fread(&(atual->Prox), sizeof(long int), 1, f);
    fread(&(atual->id), sizeof(int), 1, f);
    fread(&(atual->idade), sizeof(int), 1, f);

    // ler tamanho da string, e alocar memória desse tamanho + 1 (para escrever '\0' no final)
    fread(&(atual->tamNomeJog), sizeof(int), 1, f);
    atual->nomeJogador = (char *)calloc((atual->tamNomeJog + 1), sizeof(char));
    if (atual->tamNomeJog) {
        fread(atual->nomeJogador, atual->tamNomeJog, 1, f);
        (atual->nomeJogador)[atual->tamNomeJog] = '\0';
    }

    // ler tamanho da string, e alocar memória desse tamanho + 1 (para escrever '\0' no final)
    fread(&(atual->tamNacionalidade), sizeof(int), 1, f);
    atual->nacionalidade = (char *)calloc((atual->tamNacionalidade + 1), sizeof(char));
    if (atual->tamNacionalidade) {
        fread(atual->nacionalidade, atual->tamNacionalidade, 1, f);
        (atual->nacionalidade)[atual->tamNacionalidade] = '\0';
    }

    // ler tamanho da string, e alocar memória desse tamanho + 1 (para escrever '\0' no final)
    fread(&(atual->tamNomeClube), sizeof(int), 1, f);
    atual->nomeClube = (char *)calloc((atual->tamNomeClube + 1), sizeof(char));
    if (atual->tamNomeClube) {
        fread(atual->nomeClube, atual->tamNomeClube, 1, f);
        (atual->nomeClube)[atual->tamNomeClube] = '\0';
    }

    // o ler_linha_bin irá ler apenas as informações que preenchemos para o registro
    // caso tenha lixo de memória $$$, pois reaproveitamos o local por exemplo, ele não irá ler
    // assim o ponteiro estará na posição errada, logo devemos pular essa diferença de tamanho
    // manualmente, assim tamReg é o tamanho que de fato eu utilizei para o registro atual
    // e tamanhoRegistro é o espaço que ele está ocupando (devido a reutilização de espaço por exemplo)
    // devemos pular a diferença desses tamanhos em relação ao local apontado atualmente
    int tamReg = 33 + atual->tamNacionalidade + atual->tamNomeClube + atual->tamNomeJog;
    if (atual->tamanhoRegistro != tamReg)
        fseek(f, atual->tamanhoRegistro - tamReg, SEEK_CUR);

    return atual;
}

/*
Função para printar o registro, dado uma struct do registro atual
*/
void printar_registro(JOGADOR *atual) {

    if (atual->tamNomeJog)
        printf("Nome do Jogador: %s\n", atual->nomeJogador);
    else
        printf("Nome do Jogador: SEM DADO\n");

    if (atual->tamNacionalidade)
        printf("Nacionalidade do Jogador: %s\n", atual->nacionalidade);
    else
        printf("Nacionalidade do Jogador: SEM DADO\n");

    if (atual->tamNomeClube)
        printf("Clube do Jogador: %s\n", atual->nomeClube);
    else
        printf("Clube do Jogador: SEM DADO\n");

    printf("\n");
}

/*
Nessa função iremos ler a struct, checar se o registro foi removido
e caso o arquivo não tenha chegado no final, retornamos a struct
*/
JOGADOR *ler_linha_bin(FILE *f) {
    JOGADOR *atual = (JOGADOR *)malloc(sizeof(JOGADOR));
    if (!ler_struct(f, atual)) {
        free(atual);
        return NULL;
    }
    return atual;
}

/*
Função principal: listar todos os registros
Passos:
1. Abrir o arquivo binário para leitura binária
2. Alocar o espaço para a struct cabeçalho, e ler os campos
3. Caso exista registros no arquivo, começar a ler a linha do binário, e coloca o registro lido no atual.
4. Caso contrário, printar que não existe registro.
5. Enquanto o ponteiro *atual, resultante da leitura da linha do arquivo binário não seja nulo, continuar lendo
6. Desalocar ponteiro de cabeçalho
7. Fechar o arquivo
*/
void SELECT(char *nomeArquivoBinario) {
    FILE *fbin; // abrir/criar arquivo para escrita em binário
    if (nomeArquivoBinario == NULL || !(fbin = fopen(nomeArquivoBinario, "rb"))) {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    // aloca memoria para cabecalho, e lê os primeiros 25 bytes
    CABECALHO *cab = (CABECALHO *)malloc(sizeof(CABECALHO));
    ler_cabecalho(fbin, cab);

    if (cab->status != '1') {
        printf("Falha no processamento do arquivo.\n");
        fclose(fbin);
        return;
    }

    JOGADOR *atual; // ponteiro que vai armazenar dados
    // como o cabecalho foi lido o ponteiro está no 1o registro já

    // caso exista registros adicionados, começar a ler sequencialmente
    // caso o registro lido não esteja logicamente removido, printar
    if (cab->nroRegArq) {
        while ((atual = ler_linha_bin(fbin))) {
            if (atual->removido != '1') printar_registro(atual);
            desalocar_struct(&atual);
        }
    } else {
        printf("Registro inexistente.\n\n");
    }

    free(cab);
    fclose(fbin);
}