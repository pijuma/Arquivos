#include "escrita.h"

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
essa função serve para pular a linha inicio do arquivo
que apenas contém a legenda
*/
void pula_linha(FILE *f) {
    char c;
    while ((c = fgetc(f)) != '\n') {
    } // pula linha
}

/*
Essa função converte uma string dada de tamanho "tam" em um inteiro
*/
int getInt(char *s, int tam) {

    int ans = 0;
    int pot = 1;

    for (int i = tam - 1; i >= 0; i--) {
        ans += (s[i] - '0') * pot;
        pot *= 10;
    }

    return ans;
}

/*
essa função serve para ler o inteiro, caso exista
o inteiro é lido como string e posteriormente é convertido em um inteiro
caso não exista um inteiro na posição, será lida uma vírgula e portanto a string
lida terá tamanho 0, já que a vírgula indica o fim do inteiro que estamos lendo, assim
guardaremos -1 no inteiro, marcando que aquele campo está vazio
*/
int preencherInt(FILE *f, int *campo) {
    // aloca a memória para a string auxiliar
    char *aux = (char *)malloc(12 * sizeof(char));
    if (!aux) return 0;

    int ct = 0;
    char c;
    // caso o caracter seja uma vírgula, sai da função
    // caso o caracter seja EOF, retorna 0 (falso)
    while ((c = fgetc(f)) != ',') {
        if (c == EOF) {
            free(aux);
            return 0;
        }
        aux[ct++] = c; // lendo a idade (pode nao ter)
    }
    aux[ct] = '\0';

    // caso exista caracter, converte a string para um int
    // caso contrário, escreve -1
    if (ct)
        *campo = getInt(aux, ct);
    else
        *campo = -1;

    free(aux); // liberando a string usada
    return 1;
}

/*
essa função lê a string que começa na posição atual apontada por f
caso esteja vazia o campo estará vazio
*/
int preencherString(FILE *f, char **campo) {
    // alocar memoria para a string auxiliar
    char *aux = (char *)malloc(1024 * sizeof(char));
    if (!aux) return 0;

    int ct = 0; // contador de caracteres
    char c;
    // lê a string até encontrar uma vírgula ou uma quebra de linha
    // adição após a correção: contar caso em que tenha um '\r' também
    while ((c = fgetc(f)) != ',' && (c != '\n')) {
        if (c == '\r') {
            fgetc(f); // ler o '\n' que vem junto com o '\r'
            break;
        }
        aux[ct++] = c;
    }
    aux[ct] = '\0';

    // aloca a quantidade de memória necessária para a string
    (*campo) = (char *)malloc((ct + 1) * sizeof(char));
    if (ct) {
        for (int i = 0; i < ct; i++)
            (*campo)[i] = aux[i];
        (*campo)[ct] = '\0';
    }

    free(aux);
    return ct;
}

/*
função para ler os dados do csv e armazenas na struct JOGADOR criada
*/
JOGADOR *ler_linha(FILE *f) {

    JOGADOR *atual = (JOGADOR *)malloc(sizeof(JOGADOR));

    // a função preencherInt foi adaptada para, caso o arquivo leia a linha e
    // encontre EOF, retorne 0 (false). Caso isso seja verdade, então o final
    // do arquivo foi atingido, e a função ler_linha deve retornar NULL
    if (!preencherInt(f, &(atual->id))) {
        free(atual);
        return NULL;
    }

    preencherInt(f, &(atual->idade));

    atual->tamNomeJog = preencherString(f, &(atual->nomeJogador));
    atual->tamNacionalidade = preencherString(f, &(atual->nacionalidade));
    atual->tamNomeClube = preencherString(f, &(atual->nomeClube));

    // Tamanho do registro = Parte de tamanho fixo + strings (partes de tamanho variável)
    atual->tamanhoRegistro = 33 + atual->tamNacionalidade + atual->tamNomeClube + atual->tamNomeJog;
    atual->removido = '0'; // para esse trabalho nao temos remoção
    atual->Prox = -1;      // para esse trabalho nao utilizaremos essa informação

    return atual;
}

/*
função para escrever cada campo da struct JOGADOR, no arquivo binário
*/
void escrever_binario(FILE *f, JOGADOR *atual) {
    fwrite(&(atual->removido), sizeof(char), 1, f);
    fwrite(&(atual->tamanhoRegistro), sizeof(int), 1, f);
    fwrite(&(atual->Prox), sizeof(long int), 1, f);
    fwrite(&(atual->id), sizeof(int), 1, f);
    fwrite(&(atual->idade), sizeof(int), 1, f);
    fwrite(&(atual->tamNomeJog), sizeof(int), 1, f);
    fwrite(atual->nomeJogador, atual->tamNomeJog, 1, f);
    fwrite(&(atual->tamNacionalidade), sizeof(int), 1, f);
    fwrite(atual->nacionalidade, atual->tamNacionalidade, 1, f);
    fwrite(&(atual->tamNomeClube), sizeof(int), 1, f);
    fwrite(atual->nomeClube, atual->tamNomeClube, 1, f);
}

/*
função para escrever o cabeçalho no arquivo binário
*/
void escrever_cabecalho(FILE *f, CABECALHO *cab) {
    fwrite(&(cab->status), sizeof(char), 1, f);
    fwrite(&(cab->topo), sizeof(long int), 1, f);
    fwrite(&(cab->proxByteOffset), sizeof(long int), 1, f);
    fwrite(&(cab->nroRegArq), sizeof(int), 1, f);
    fwrite(&(cab->nroRegRem), sizeof(int), 1, f);
}

/*
função para inicializar o cabeçalho - vazio
*/
void inicializar_cab(CABECALHO *cab) {
    cab->status = '0';
    cab->topo = -1;
    cab->proxByteOffset = 0;
    cab->nroRegArq = 0;
    cab->nroRegRem = 0;
}

/*
função para desalocar os dados da struct, as strings que são
alocadas dinamicamente e a própria struct
*/
void desalocar_struct(JOGADOR **atual) {
    if (*atual == NULL) return;

    free((*atual)->nomeJogador);
    free((*atual)->nomeClube);
    free((*atual)->nacionalidade);

    free(*atual);
    *atual = NULL;
}

/*
função que irá escrever os dados do .csv para o .bin
ela se utiliza das funções feitas anteriormente
*/
void CREATE_TABLE(char *nomeCsv, char *nomeArquivoBinario) {

    FILE *fbin = fopen(nomeArquivoBinario, "wb"); // abrir/criar arquivo para escrita em binário
    if (nomeArquivoBinario == NULL || (fbin == NULL)) {
        printf("ERRO AO ESCREVER O BINARIO : não foi possível abrir o arquivo que me passou para escrita. \n");
        return;
    }

    FILE *fcsv = fopen(nomeCsv, "r"); // abrir arquivo para leitura em texto (csv)
    if (nomeCsv == NULL || (fcsv == NULL)) {
        printf("ERRO AO LER O CSV : não foi possível abrir o arquivo que me passou para leitura. \n");
        return;
    }

    CABECALHO *cab = (CABECALHO *)malloc(sizeof(CABECALHO));
    inicializar_cab(cab); // inicializa cabeçalho como nulo

    pula_linha(fcsv);              // devemos pular a primeira linha do csv, pois apenas consta a legenda
    escrever_cabecalho(fbin, cab); // pula 25 bytes (tamanho fixo do registro de cabeçalho)
    cab->proxByteOffset += 25;

    // ler_linha = lê a linha correspondente ao jogador da vez, e coloca em atual
    // escrever_binario = escreve os campos do jogador no arquivo binário
    JOGADOR *atual; // ponteiro que vai armazenar dados
    while ((atual = ler_linha(fcsv))) {
        escrever_binario(fbin, atual);
        cab->proxByteOffset += atual->tamanhoRegistro;
        desalocar_struct(&atual);
        cab->nroRegArq++;
    }

    cab->status = '1'; // atualizar o status porque conseguimos escrever no binário corretamente

    fseek(fbin, 0, SEEK_SET); // volta para o inicio do arquivo para atualizar o cabeçalho
    escrever_cabecalho(fbin, cab);
    free(cab);

    // fecha os arquivos
    fclose(fbin);
    fclose(fcsv);
}
