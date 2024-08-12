#include "remocao.h"

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

struct item_ {
    int id;
    long int byteoffset;
};

/*
função para checar se o jogador que estamos olhando atualmente bate com os dados procurados
essa função será utilizada na remoção
assim, caso o jogador esteja dentro dos critérios de remoção, ao final, iremos removê-lo
vale ressaltar que só usamos essa função caso o ID  não seja um dos critérios de remoção
*/
int comparar_remover(JOGADOR *busca, JOGADOR *atual, FILE *fbin, CABECALHO *cab, ITEM **array, int tamArray) {
    if (busca->id != -1 && busca->id != atual->id)
        return 0;
    if (busca->idade != -1 && busca->idade != atual->idade)
        return 0;
    if (busca->tamNomeJog != -1 && busca->nomeJogador != NULL && atual->nomeJogador != NULL)
        if (strcmp(busca->nomeJogador, atual->nomeJogador))
            return 0;
    if (busca->tamNacionalidade != -1 && busca->nacionalidade != NULL && atual->nacionalidade != NULL)
        if (strcmp(busca->nacionalidade, atual->nacionalidade))
            return 0;
    if (busca->tamNomeClube != -1 && busca->nomeClube != NULL && atual->nomeClube != NULL)
        if (strcmp(busca->nomeClube, atual->nomeClube))
            return 0;

    // achamos o jogador, mas nao temos o byteoffset dele, precisamos achar.
    // para isso fazemos busca binaria no arquivo de indices, para achar o byteoffset
    // do registro que estamos olhando

    ITEM *apagar = busca_indice(array, tamArray, atual->id);
    // Tendo o byteoffset, é facil remover o registro
    remover_registro(fbin, apagar->byteoffset, cab);

    // marcar como removido no vetor
    // O item "apagar" não foi de fato removido do array e também não foi setado
    // como NULL para que a busca binária possa continuar acontecendo.
    // O byteoffset dele foi setado como -1 para que, na hora de inserir no arquivo
    // de índices, ele não considerar o item que foi removido.
    apagar->byteoffset = -1;
    return 1; // para sinalizar que removemos o registro
}

/*
nessa função iremos remover o registro e adicioná-lo da devida forma
na pilha de reuso, mantendo essa de forma ordenada pelo tamanho para a inserção pelo best fit
tornar-se mais fácil
*/
void remover_registro(FILE *fbin, long int byteoffset, CABECALHO *cab) {
    // fseek para o byteoffset do registro que será removido
    fseek(fbin, byteoffset, SEEK_SET);

    // alocar memória e ler o registro que será removido no arquivo de dados
    JOGADOR *registroRemover = (JOGADOR *)malloc(sizeof(JOGADOR));
    ler_struct(fbin, registroRemover);

    // marcar como removido
    registroRemover->removido = '1';

    // pegar o tamanho do registro que será removido
    int tamReg = registroRemover->tamanhoRegistro;
    // pegar o byteoffset do primeiro elemento da pilha de removidos
    // iremos percorrer a pilha para acharmos a posição que o novo removido
    // será adicionado
    long int topo = cab->topo;

    if (topo == -1) { // vazia -> sou o 1o removido
        // para o primeiro removido, basta setar o byteoffset do topo para
        // o byteoffset do novo registro removido e também setar o "Prox" do
        // registro removido como -1

        cab->topo = byteoffset;
        registroRemover->Prox = -1;
    } else {
        // se não for o primeiro removido, então devemos comparar os
        // registros e encontrar a posição correta do registro na lista encadeada

        // pegar o primeiro elemento removido (topo)
        fseek(fbin, topo, SEEK_SET);
        JOGADOR *atual = (JOGADOR *)malloc(sizeof(JOGADOR));
        ler_struct(fbin, atual);

        // Sou o novo topo (caso o que quero remover seja menor que
        // o primeiro registro, então substituo o topo)
        if (atual->tamanhoRegistro >= tamReg) {
            // para substituir o topo, basta setar o próximo do registro
            // a ser removido como o topo antigo,
            // e setar o topo como o byteoffset do registro a ser removido
            registroRemover->Prox = topo;
            cab->topo = byteoffset;

            // é necessário desalocar a struct para evitar memory leak
            desalocar_struct(&atual);
        } else {
            // caso contrário, então devo percorrer a lista encadeada até
            // achar a posição adequada para ele.
            long int byteoffset_atual = topo;
            while (1) {
                // caso o próximo seja -1, então cheguei no final da lista
                // encadeada, e coloco o registro a remover no final
                if (atual->Prox == -1) {
                    // sou o proximo removido em relação ao ultimo registro removido da pilha
                    // pois sou o novo final da pilha
                    atual->Prox = byteoffset;
                    fseek(fbin, byteoffset_atual, SEEK_SET);
                    escrever_binario(fbin, atual);

                    // nao tem nenhum removido depois de mim
                    registroRemover->Prox = -1;

                    // desalocar o espaço de memória para evitar memory leak
                    desalocar_struct(&atual);

                    break;
                }

                // caso não seja o último, então devo acessar o próximo elemento
                // da lista encadeada de registros removidos.
                long int byteoffset_prox = atual->Prox;
                fseek(fbin, byteoffset_prox, SEEK_SET);
                JOGADOR *next = (JOGADOR *)malloc(sizeof(JOGADOR));
                ler_struct(fbin, next);

                // Com o próximo registro removido, basta compará-lo com o tamanho do
                // registro a ser removido.
                // se o tamanho do que eu vou remover está entre o tamanho do atual e do proximo
                // devo inserir o meu entre esses registros
                if (atual->tamanhoRegistro < tamReg && next->tamanhoRegistro >= tamReg) {
                    // proximo do registro que estou removendo é o proximo do atual
                    registroRemover->Prox = atual->Prox;
                    // proximo do meu atual sou eu
                    atual->Prox = byteoffset;
                    fseek(fbin, byteoffset_atual, SEEK_SET);
                    escrever_binario(fbin, atual);

                    // desalocar os espaços de memória para evitar memory leak
                    desalocar_struct(&atual);
                    desalocar_struct(&next);

                    break;
                }

                desalocar_struct(&atual);
                atual = next;
                byteoffset_atual = byteoffset_prox;
            }
        }
    }

    // reescrevendo o registro removido no arquivo de dados
    // com os dados atualizados
    fseek(fbin, byteoffset, SEEK_SET);
    escrever_binario(fbin, registroRemover);

    desalocar_struct(&registroRemover);
}

/*
Essa é a função principal desse arquivo, onde usaremos as demais funções implementadas
nessa iremos abrir tanto o arquivo de dados quanto o binário e criar o arquivo de indices para efetuar remoções de registros
assim iremos fazer todas as operações necessárias para ao final o arquivo de indices estar gravado
corretamente (sem removidos) e o arquivo de dados também, com a pilha de removidos ordenada
*/

void DELETE(char *nomeArquivoBinario, char *nomeArquivoIndice) {

    // criar arquivo de indice
    CREATE_INDEX(nomeArquivoBinario, nomeArquivoIndice);

    FILE *fbin; // abrir/criar arquivo para LEITURA/ESCRITA em binário
    if (nomeArquivoBinario == NULL || !(fbin = fopen(nomeArquivoBinario, "rb+"))) {
        printf("ERRO AO ESCREVER O BINARIO : não foi possível abrir o arquivo que me passou para escrita. \n");
        return;
    }

    FILE *findex = fopen(nomeArquivoIndice, "rb"); // abrir/criar arquivo para LEITURA em binário
    if (nomeArquivoIndice == NULL || (findex == NULL)) {
        printf("ERRO AO LER O BINARIO : não foi possível abrir o arquivo que me passou para escrita. \n");
        return;
    }

    // OPERAÇÕES INICIAIS COM O ARQUIVO BINÁRIO DE DADOS
    // aloca memoria para cabecalho, e lê os primeiros 25 bytes
    CABECALHO *cab = (CABECALHO *)malloc(sizeof(CABECALHO));
    ler_cabecalho(fbin, cab);

    // caso o status não seja 1, então o arquivo está inválido
    if (cab->status != '1') {
        printf("Falha no processamento do arquivo.\n");
        free(cab);
        fclose(fbin);
        fclose(findex);
        return;
    }

    // OPERAÇÕES INICIAIS COM O ARQUIVO DE ÍNDICE
    // checar se o arquivo de index está certo para ser utilizado
    char status;
    fread(&status, sizeof(char), 1, findex);
    if (status != '1') {
        printf("Falha no processamento do arquivo.\n");
        free(cab);
        fclose(fbin);
        fclose(findex);
        return;
    }

    // criar array para busca binária
    ITEM **array = cria_array(findex, cab->nroRegArq);
    int tamArray = cab->nroRegArq;
    // fechar o arquivo de índice
    fclose(findex);

    // abrir novamente, mas para escrita
    // preciso abrir novamente pois quero reescrever o arquivo de índices
    findex = fopen(nomeArquivoIndice, "wb"); // abrir/criar arquivo para escrita em binário
    if (nomeArquivoIndice == NULL || (findex == NULL)) {
        printf("ERRO AO ESCREVER O BINARIO : não foi possível abrir o arquivo que me passou para escrita. \n");
        return;
    }

    status = '0';
    cab->status = '0';
    // escrever que o status do arquivo de índices é 0
    fseek(findex, 0, SEEK_SET);
    fwrite(&status, sizeof(char), 1, findex);

    // escrever que o status do arquivo de dados é 0
    fseek(fbin, 0, SEEK_SET);
    escrever_cabecalho(fbin, cab);

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

        // como o cabecalho foi lido o ponteiro está no 1o registro já
        // caso existam registros, começar a ler.
        if (cab->nroRegArq) {
            // se um dos criterios de busca é o ID, só iremos remover um registro
            if (busca->id != -1) {
                ITEM *apagar = busca_indice(array, tamArray, busca->id);
                if (apagar != NULL) {
                    remover_registro(fbin, apagar->byteoffset, cab);
                    cab->nroRegArq--;
                    cab->nroRegRem++;
                    // marcar como removido no vetor
                    apagar->byteoffset = -1;
                }
            } else {
                int qtdRem = 0;
                while ((atual = ler_linha_bin(fbin))) {
                    // se o atual não foi removido ainda, checamos se deve ser removido
                    // a função comparar_remover retorna 1 caso o registro "atual" seja removido
                    if (atual->removido != '1') {
                        qtdRem += comparar_remover(busca, atual, fbin, cab, array, tamArray);
                    }
                    desalocar_struct(&atual);
                }
                desalocar_necessario(busca);
                // atualizando a quantidade de registros removidos e não removidos
                cab->nroRegArq -= qtdRem;
                cab->nroRegRem += qtdRem;
            }
            // se não for a última busca a fazer, voltar o ponteiro para o início do arquivo
            if (i != q)
                fseek(fbin, 25, SEEK_SET);
        }
    }

    // reescrever cabeçalho com os dados atualizados
    cab->status = '1';
    fseek(fbin, 0, SEEK_SET);
    escrever_cabecalho(fbin, cab);

    // reescrever arquivo de índice com a lista encadeada atualizada pós remoções
    LISTA *reg = passar_vetor_indice(array, tamArray);
    status = '1';
    escrever_arquivo_indice(findex, reg, status);

    // desalocar o array criado
    desaloca_array(&array, tamArray);
    lista_apagar(&reg);

    // desalocar todas as structs necessárias
    free(busca);
    free(cab);
    fclose(fbin);
    fclose(findex);

    binarioNaTela(nomeArquivoBinario);
    binarioNaTela(nomeArquivoIndice);
}