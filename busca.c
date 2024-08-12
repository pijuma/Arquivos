#include "busca.h"

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
struct jogador / busca:

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
inicializamos a struct da busca com tudo -1 para preencher posteriormente
apenas os campos que iremos usar para busca, para saber qual campo queremos ou não buscar.

Os inteiros serão tidos como -1 caso não queira buscar aquele campo.
Caso não queira buscar a string, o tamanho dela será -1.
*/
void inicializar_null(JOGADOR *busca) {
    busca->removido = -1;
    busca->tamanhoRegistro = -1;
    busca->Prox = -1;
    busca->id = -1;
    busca->idade = -1;
    busca->tamanhoRegistro = busca->tamNacionalidade = busca->tamNomeClube = busca->tamNomeJog = -1;
    busca->nacionalidade = NULL;
    busca->nomeJogador = NULL;
    busca->nomeClube = NULL;
}

/*
Função para desalocar o que for necessário do registro de busca.
Caso exista uma string alocada, então o tam dele será igual a 1.
*/
void desalocar_necessario(JOGADOR *busca) {
    if (busca->tamNomeJog == 1)
        free(busca->nomeJogador);
    if (busca->tamNomeClube == 1)
        free(busca->nomeClube);
    if (busca->tamNacionalidade == 1)
        free(busca->nacionalidade);
}

/*
comparar cada campo com -1.
Caso não seja, então eu quero comparar o campo com o de busca.
Caso sejam diferentes, então não é o que eu quero achar, e sai da função sem printar
Retorna 0 caso o registro não seja o que se quer
Retorna 1 caso o registro seja o que se quer
*/
int comparar(JOGADOR *busca, JOGADOR *atual, int *qtdRes) {
    if (busca->id != -1 && busca->id != atual->id)
        return 0;
    if (busca->idade != -1 && busca->idade != atual->idade)
        return 0;
    if (busca->tamNomeJog != -1)
        if (strcmp(busca->nomeJogador, atual->nomeJogador))
            return 0;
    if (busca->tamNacionalidade != -1)
        if (strcmp(busca->nacionalidade, atual->nacionalidade))
            return 0;
    if (busca->tamNomeClube != -1)
        if (strcmp(busca->nomeClube, atual->nomeClube))
            return 0;
    printar_registro(atual);
    (*qtdRes)++;
    return 1;
}

/*
Função para ler o critério de busca, caso exista
*/
void ler_criterio(JOGADOR *busca) {
    char op[20];
    scanf("%s", op);
    if (!strcmp(op, "id")) {
        int id;
        scanf("%d", &id);
        busca->id = id;
    } else if (!strcmp(op, "idade")) {
        int idade;
        scanf("%d", &idade);
        busca->idade = idade;
    } else if (!strcmp(op, "nacionalidade")) {
        char *str = (char *)malloc(1024 * sizeof(char));
        scan_quote_string(str);
        busca->tamNacionalidade = 1;
        busca->nacionalidade = str;
    } else if (!strcmp(op, "nomeJogador")) {
        char *str = (char *)malloc(1024 * sizeof(char));
        scan_quote_string(str);
        busca->tamNomeJog = 1;
        busca->nomeJogador = str;
    } else if (!strcmp(op, "nomeClube")) {
        char *str = (char *)malloc(1024 * sizeof(char));
        scan_quote_string(str);
        busca->tamNomeClube = 1;
        busca->nomeClube = str;
    }
}

/*
Função principal: buscar jogadores com critérios específicos do arquivo binário criado.
Passos:
1. abrir o arquivo binario para leitura
2. alocar espaço de memória para cabeçalho, e ler do arquivo binário
3. ler a quantidade de buscas de jogadores que será realizado
4. para cada busca, ler a quantidade de critérios
5. ler o critério para a quantidade de buscas, e preencher a struct de busca
6. buscar. Caso um dos critérios seja id, quando encontrar a id, sair imediatamente da busca
7. desalocar.
*/
void WHERE(char *nomeArquivoBinario) {
    FILE *fbin; // abrir/criar arquivo para leitura em binário
    if (nomeArquivoBinario == NULL || !(fbin = fopen(nomeArquivoBinario, "rb"))) {
        printf("ERRO AO LER O BINARIO : não foi possível abrir o arquivo que me passou para leitura. \n");
        return;
    }

    // aloca memoria para cabecalho, e lê os primeiros 25 bytes
    CABECALHO *cab = (CABECALHO *)malloc(sizeof(CABECALHO));
    ler_cabecalho(fbin, cab);

    if (cab->status != '1') {
        printf("Falha no processamento do arquivo.\n");
        return;
    }

    int q; // quantidade de buscas
    scanf("%d", &q);
    JOGADOR *busca = (JOGADOR *)malloc(sizeof(JOGADOR));

    for (int i = 1; i <= q; i++) {
        int n;
        scanf("%d ", &n);
        inicializar_null(busca);
        for (int j = 1; j <= n; j++) {
            ler_criterio(busca);
        }

        JOGADOR *atual;  // ponteiro que vai armazenar dados do registro atual
        int qtdRes = 0;

        printf("Busca %d\n\n", i);

        // como o cabecalho foi lido o ponteiro está no 1o registro já
        // caso existam registros, começar a ler. 
        if (cab->nroRegArq)
            while ((atual = ler_linha_bin(fbin))) {
                // caso a leitura atual não esteja removido, prossegue
                // caso compare e tenha dado resultado positivo, e um dos critérios esteja no id, 
                // então interrompe a busca pois só existe 1 resultado em que o id será igual.
                if (atual->removido != '1')
                    if (comparar(busca, atual, &qtdRes) && busca->id != -1) {
                        desalocar_struct(&atual);
                        break;
                    }
                desalocar_struct(&atual);
            }
        
        // caso não tenha encontrado, ou caso não existam registros no arquivo binário, printa o abaixo:
        if (qtdRes == 0) printf("Registro inexistente.\n\n");

        desalocar_necessario(busca);

        // se não for a última busca a fazer, voltar o ponteiro para o início do arquivo
        if (i != q)
            fseek(fbin, 25, SEEK_SET);
    }

    // desalocar todas as structs necessárias
    free(busca);
    free(cab);
    fclose(fbin);
}
