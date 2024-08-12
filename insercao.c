#include "insercao.h"
#include "lista.h"

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
função para pegar as entradas do jogador que será inserido no arquivo
e retornar a struct
*/
JOGADOR *ler_jogador() {
    // alocar espaço para a struct
    JOGADOR *registro_novo = (JOGADOR *)malloc(sizeof(JOGADOR));
    // como vamos inserir, o registro não está removido, e o próximo dele será -1
    registro_novo->removido = '0';
    registro_novo->Prox = -1;

    // ler o id
    scanf("%d", &(registro_novo->id));

    // ler a idade
    // É possível que a string seja NULO. Nesse caso, o scan_quote_string retorna
    // uma string vazia. Nesse caso, passar o valor vazio.
    char *idade = (char *)malloc(64 * sizeof(char));
    scan_quote_string(idade);
    if (strcmp(idade, "") != 0)
        registro_novo->idade = getInt(idade, strlen(idade));
    else
        registro_novo->idade = -1;
    free(idade);

    // ler o nome do jogador
    // É possível que a string seja NULO. Nesse caso, o scan_quote_string retorna
    // uma string vazia. Nesse caso, passar o valor vazio.
    char *nomeJog = (char *)malloc(64 * sizeof(char));
    scan_quote_string(nomeJog);
    if (strcmp(nomeJog, "") != 0)
        registro_novo->tamNomeJog = strlen(nomeJog);
    else
        registro_novo->tamNomeJog = 0;
    registro_novo->nomeJogador = nomeJog;

    // ler a nacionalidade do jogador
    // É possível que a string seja NULO. Nesse caso, o scan_quote_string retorna
    // uma string vazia. Nesse caso, passar o valor vazio.
    char *nacionalidade = (char *)malloc(64 * sizeof(char));
    scan_quote_string(nacionalidade);
    if (strcmp(nacionalidade, "") != 0)
        registro_novo->tamNacionalidade = strlen(nacionalidade);
    else
        registro_novo->tamNacionalidade = 0;
    registro_novo->nacionalidade = nacionalidade;

    // ler o nome do clube do jogador
    // É possível que a string seja NULO. Nesse caso, o scan_quote_string retorna
    // uma string vazia. Nesse caso, passar o valor vazio.
    char *nomeClube = (char *)malloc(64 * sizeof(char));
    scan_quote_string(nomeClube);
    if (strcmp(nomeClube, "") != 0)
        registro_novo->tamNomeClube = strlen(nomeClube);
    else
        registro_novo->tamNomeClube = 0;
    registro_novo->nomeClube = nomeClube;

    // calcular o tamanho do registro novo
    registro_novo->tamanhoRegistro = 33 +
                                     registro_novo->tamNacionalidade +
                                     registro_novo->tamNomeClube +
                                     registro_novo->tamNomeJog;

    return registro_novo;
}

/*
Função para inserir no espaço que melhor couber no arquivo de dados
Existem alguns casos:
1. Caso não tenha registros removidos, basta inserir no final (no cab->proxByteOffset)
2. Caso o novo registro seja menor que o registro no topo, então substituo o topo pelo próximo dos removidos
3. Caso não seja nenhum dos dois casos, percorrer a lista encadeada de removidos, e inserir no local adequado
4. Caso percorra e chegue no final, inserir no final do arquivo (no cab->proxByteoffset)
*/
long int inserir_best_fit(FILE *fbin, JOGADOR *novo, CABECALHO *cab) {
    long int byteoffset_inserido;
    // caso não tenha registros removidos, inserir no final
    if (cab->nroRegRem == 0) {
        // inserir no final do arquivo, já que não existe registro removido
        // pode ser também que todos os espaços removidos já foram reutilizados
        fseek(fbin, cab->proxByteOffset, SEEK_SET);
        escrever_binario(fbin, novo);
        byteoffset_inserido = cab->proxByteOffset;

        // atualizar o proxByteOffset
        cab->proxByteOffset += novo->tamanhoRegistro;
        return byteoffset_inserido;
    }

    // pegar o registro do topo
    long int byteoffset_atual = cab->topo;
    fseek(fbin, byteoffset_atual, SEEK_SET);
    JOGADOR *atual = (JOGADOR *)malloc(sizeof(JOGADOR));
    ler_struct(fbin, atual);

    // como o registro não foi removido, retorna um erro
    if (atual->removido == '0') {
        printf("Falha no processamento do arquivo.\n");
        return -1;
    }

    // caso o novo seja menor que o tamanho do topo,
    // então insiro no primeiro, e mudo o topo para o byteoffset do proximo
    if (novo->tamanhoRegistro < atual->tamanhoRegistro) {
        // calcular o resto para depois substituir por '$'
        int resto = atual->tamanhoRegistro - novo->tamanhoRegistro;

        // O tamanho do registro novo será igual ao tamanho do que substituirá, pois
        // o espaço remanescente será substituído por '$'
        novo->tamanhoRegistro = atual->tamanhoRegistro;
        // inserir no topo dos registros removidos
        fseek(fbin, cab->topo, SEEK_SET);
        escrever_binario(fbin, novo);
        byteoffset_inserido = cab->topo;

        // substituir por '$'
        char x = '$';
        while (resto--)
            fwrite(&x, 1, sizeof(char), fbin);

        // Atualizar o valor do topo, e diminuir a quantidade de removidos
        cab->topo = atual->Prox;
        cab->nroRegRem--;

        desalocar_struct(&atual);
    } else {
        // percorrer até encontrar o local certo
        while (1) {
            // caso chegue no final, então insiro no final do arquivo, já que
            // o novo registro é maior que todos os removidos
            if (atual->Prox == -1) {
                // inserir no final do arquivo
                fseek(fbin, cab->proxByteOffset, SEEK_SET);
                escrever_binario(fbin, novo);
                byteoffset_inserido = cab->proxByteOffset;

                // atualizar o proxByteOffset do cabeçalho
                cab->proxByteOffset += novo->tamanhoRegistro;
                break;
            }

            // pegar o próximo registro logicamente removido
            long int byteoffset_prox = atual->Prox;
            fseek(fbin, byteoffset_prox, SEEK_SET);
            JOGADOR *next = (JOGADOR *)malloc(sizeof(JOGADOR));
            ler_struct(fbin, next);

            // Caso o novo registro seja maior que o atual, e menor que o próximo,
            // então encontrei o espaço em que preciso substituir
            if (novo->tamanhoRegistro > atual->tamanhoRegistro &&
                novo->tamanhoRegistro <= next->tamanhoRegistro) {
                // inserir entre, e atualizar os registros envolvidos
                // quero inserir o novo no espaço do "next"

                // Como o "next" vai passar a ser reutilizado, preciso apontar o próximo
                // do atual como o próximo do next (já que é uma lista encadeada)
                atual->Prox = next->Prox;
                // apontar para onde quero reescrever o atual
                fseek(fbin, byteoffset_atual, SEEK_SET);
                escrever_binario(fbin, atual);

                // calcular o resto para depois substituir por '$'
                int resto = next->tamanhoRegistro - novo->tamanhoRegistro;

                // o tamanho do registro novo será igual ao que vai ser substituído
                // pois o espaço remanescente está substituído por '$'
                novo->tamanhoRegistro = next->tamanhoRegistro;
                // reescrever o next pelo novo
                fseek(fbin, byteoffset_prox, SEEK_SET);
                escrever_binario(fbin, novo);
                byteoffset_inserido = byteoffset_prox;

                // substituir por '$'
                char x = '$';
                while (resto--)
                    fwrite(&x, 1, sizeof(char), fbin);

                // como ocupei um espaço de um registro removido, então
                // diminuirei o número de registros removidos.
                cab->nroRegRem--;

                desalocar_struct(&atual);
                desalocar_struct(&next);

                break;
            }

            // desalocar o atual, e mudar o valor de atual para o next
            desalocar_struct(&atual);
            atual = next;
            byteoffset_atual = byteoffset_prox;
        }
    }
    return byteoffset_inserido;
}

/*
Função principal para inserir registros novos no arquivo de dados
e atualizar o arquivo de índices
*/
void INSERT_INTO(char *nomeArquivoBinario, char *nomeArquivoIndice) {

    // criar arquivo de indice
    CREATE_INDEX(nomeArquivoBinario, nomeArquivoIndice);

    FILE *fbin; // abrir/criar arquivo para escrita e leitura em binário
    if (nomeArquivoBinario == NULL || !(fbin = fopen(nomeArquivoBinario, "rb+"))) {
        printf("ERRO AO ESCREVER O BINARIO : não foi possível abrir o arquivo que me passou para escrita. \n");
        return;
    }

    FILE *findex; // abrir/criar arquivo para escrita e leitura em binário
    if (nomeArquivoIndice == NULL || !(findex = fopen(nomeArquivoIndice, "rb+"))) {
        printf("ERRO AO ESCREVER O BINARIO : não foi possível abrir o arquivo que me passou para escrita. \n");
        return;
    }

    // OPERAÇÕES INICIAIS COM O ARQUIVO BINÁRIO DE DADOS
    // aloca memoria para cabecalho, e lê os primeiros 25 bytes
    CABECALHO *cab = (CABECALHO *)malloc(sizeof(CABECALHO));
    ler_cabecalho(fbin, cab);
    // caso o status não seja 1, então o arquivo está com problemas
    if (cab->status != '1') {
        printf("Falha no processamento do arquivo.\n");
        free(cab);
        fclose(fbin);
        fclose(findex);
        return;
    }

    // OPERAÇÕES INICIAIS COM O ARQUIVO DE ÍNDICE
    // checar se o arquivo de index está bem para ser utilizado
    char status;
    fread(&status, sizeof(char), 1, findex);
    if (status != '1') {
        printf("Falha no processamento do arquivo.\n");
        free(cab);
        fclose(fbin);
        fclose(findex);
        return;
    }

    // carregar uma lista com os dados do indice
    LISTA *reg = pegar_indice(findex);

    status = '0';
    cab->status = '0';
    // escrever que o status do arquivo de índices é 0
    fseek(findex, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, findex);

    // escrever que o status do arquivo de dados é 0
    fseek(fbin, 0, SEEK_SET);
    escrever_cabecalho(fbin, cab);

    // ler a quantidade de insercoes
    int n;
    scanf("%d", &n);

    // inserir os n registros
    // fluxo:
    // 1. ler o registro
    // 2. inserir best fit no arquivo de dados
    // 3. aumentar o número de registros adicionados
    // 4. desalocar o registro lido
    while (n--) {
        JOGADOR *novo = ler_jogador();
        long int byteoffset = inserir_best_fit(fbin, novo, cab);
        inserir_lista(reg, novo, byteoffset);
        cab->nroRegArq++;
        desalocar_struct(&novo);
    }

    // reescrever cabeçalho com os dados atualizados
    cab->status = '1';
    fseek(fbin, 0, SEEK_SET);
    escrever_cabecalho(fbin, cab);

    // reescrever o arquivo de índice com os dados atualizados
    status = '1';
    escrever_arquivo_indice(findex, reg, status);

    // desalocar a lista, o cabeçalho
    lista_apagar(&reg);
    free(cab);

    // fechar o arquivo de dados e o arquivo de índice
    fclose(fbin);
    fclose(findex);

    // caso tenha chegado até o final, então posso printar os binários
    // do arquivo de dados e do arquivo de índices
    binarioNaTela(nomeArquivoBinario);
    binarioNaTela(nomeArquivoIndice);
}