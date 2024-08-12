#include "arvb.h"

// cada registro tem tamanho fixo = 60

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
alturaNo -> guarda a altura do nó
NroChaves -> quantidade de chaves que aquele nó guarda
(no max 3)
C[i] = valor da chave i
PR[i] = byteoffset da chave i
P[] = ponteiros para os filhos -> guarda o RRN do nó
*/
struct no {
    int alturaNo;
    int NroChaves;
    int C[4];       // C[1] = C1, C2, C3
    long int PR[4]; // PR1, PR2, PR3
    int P[5];       // P1, P2, P3, P4
};

/*
utilizado na função de inserção para manter os retornos
e adicionar possiveis chaves em nós acima caso haja promoção
para isso guardamos:
id, byteoffset (da chave a ser promovida)
ponteiros dos novos filhos (caso tenha)
um bool que guarda se houve promoção ou não
*/
struct no_aux {
    int C;          // valor id
    long int PR;    // byteoffset
    int Pesq, Pdir; // ponteiro = rrn filho direita (novo se tiver)
};

// Função para inicializar cabeçalho
/*
Preenchemos tudo com valores nulos e o lixo com '$' como é pedido no trabalho
*/
CABECALHO_AB *inicializar_cabecalho_ab() {
    CABECALHO_AB *cab = (CABECALHO_AB *)malloc(sizeof(CABECALHO_AB));
    cab->status = '0';
    cab->noRaiz = 0;
    cab->proxRRN = 0;
    cab->nroChaves = 0;
    for (int i = 0; i < 47; i++)
        cab->lixo[i] = '$';
    return cab;
}

/*
Função para inicializar o nó
Alocamos a memória para o nó e settamos os valores como não existentes (-1)
e a quantidade de chaves como 0, não tem nenhuma chave ainda
*/
NO_AB *inicializar_no() {
    NO_AB *no = (NO_AB *)malloc(sizeof(NO_AB));
    no->alturaNo = -1;
    no->NroChaves = 0;
    for (int i = 0; i < 4; i++) {
        no->C[i] = -1;
        no->PR[i] = -1;
    }
    for (int i = 0; i < 5; i++) {
        no->P[i] = -1;
    }
    return no;
}

/*
Função para inicializar o nó auxiliar como nulo
*/
NO_AUX *criar_aux_null() {
    NO_AUX *novo = (NO_AUX *)malloc(sizeof(NO_AUX));
    novo->C = -1;
    novo->PR = -1;
    novo->Pdir = -1;
    novo->Pesq = -1;
    return novo;
}

/*
Função para ler o cabeçalho do arquivo
*/
CABECALHO_AB *ler_cabecalho_ab(FILE *f) {
    fseek(f, 0, SEEK_SET); // garantir que o ponteiro está no começo do arquivo
    CABECALHO_AB *cab = (CABECALHO_AB *)malloc(sizeof(CABECALHO_AB));
    fread(&(cab->status), sizeof(char), 1, f);
    fread(&(cab->noRaiz), sizeof(int), 1, f);
    fread(&(cab->proxRRN), sizeof(int), 1, f);
    fread(&(cab->nroChaves), sizeof(int), 1, f);
    fread(&(cab->lixo), sizeof(char), 47, f);
    return cab;
}

/*
Função para escrever os dados do cabeçalho no arquivo.
*/
void escrever_cabecalho_ab(FILE *f, CABECALHO_AB *cab) {
    fseek(f, 0, SEEK_SET); // garantir que o ponteiro está no começo do arquivo
    fwrite(&(cab->status), sizeof(char), 1, f);
    fwrite(&(cab->noRaiz), sizeof(int), 1, f);
    fwrite(&(cab->proxRRN), sizeof(int), 1, f);
    fwrite(&(cab->nroChaves), sizeof(int), 1, f);
    fwrite(&(cab->lixo), sizeof(char), 47, f);
}

/*
Função para criar uma nova raiz guardamos o id e byteoffset do nó e settamos o resto como nulo
*/
void ab_criar_raiz(FILE *f, int id, long int byteoffset, CABECALHO_AB *cab) {
    fseek(f, (cab->proxRRN + 1) * 60, SEEK_SET);  // proxRRN*60 + 60(pular cabeçalho)
    NO_AB *novo = (NO_AB *)malloc(sizeof(NO_AB)); // alocar memoria
    novo->alturaNo = 0;
    novo->NroChaves = 1;
    novo->C[1] = id;
    novo->C[2] = novo->C[3] = -1; // só tem uma chave no nó
    novo->PR[1] = byteoffset;
    novo->PR[2] = novo->PR[3] = -1; // só tem uma chave no nó
    novo->P[1] = novo->P[2] = novo->P[3] = novo->P[4] = -1;
    // escrever no arquivo

    // dummy (apenas para inicializar e tirar os erros do valgrind)
    novo->C[0] = novo->PR[0] = novo->P[0] = -1;

    ab_escrever_no(f, cab->proxRRN, novo);
    free(novo);

    cab->noRaiz = cab->proxRRN; // temos uma nova raiz, o vertice atual criado
    cab->proxRRN++;             // já usamos o RRN atual
}

/*
Função que administra a inserção.
Caso não tenha nós ainda na arvore, precisamos criar uma raiz.
Caso contrario devemos inserir de forma recursiva para realizar as promoções necessárias.
Em ambos casos estamos colocamos uma chave a mais (nroChaves++)
*/
void ab_inserir(FILE *findex, int id, long int byteoffset, CABECALHO_AB *cab) {
    if (!cab->nroChaves) {
        ab_criar_raiz(findex, id, byteoffset, cab);
        cab->nroChaves++;
    } else {
        ab_inserir_recur(findex, cab->noRaiz, id, byteoffset, cab);
        cab->nroChaves++;
    }
}

// Função de inserção recursiva (ela chama uma função que de fato altera os valores e ponteiros)
// Essa função serve apenas para lidar com a passagem de vértices pós split e com a inserção de chaves nos nós
NO_AUX *ab_inserir_recur(FILE *f, int RRN_atual, int id, long int byteoffset, CABECALHO_AB *cab) {
    // ler o nó atual (possui RRN_atual como local onde está escrito)
    NO_AB *atual = ab_ler_no(f, RRN_atual);
    // inicializamos para caso não tenha retorno algum
    NO_AUX *retorno = NULL; // pega retorno da funcao ou cria um pra passar pro pai

    // descer em algum filho (caso tenha)
    for (int i = 1; i <= atual->NroChaves; i++) {
        if (i == 1 && id < atual->C[1] && atual->P[1] != -1) {
            // caso esteja no primeiro elemento, seja menor que ele e exista ponteiro para ele, desce na recursão
            if ((retorno = ab_inserir_recur(f, atual->P[1], id, byteoffset, cab)) == NULL) {
                // caso retorne NULL, então o add_novo_no não fez split
                free(atual);
                return NULL;
            }
            break;
        } else if (id > atual->C[i] && (atual->C[i + 1] == -1 || id < atual->C[i + 1]) && atual->P[i + 1] != -1) {
            // caso exista ponteiros para os proximos andares, passa para o prox andar
            if ((retorno = ab_inserir_recur(f, atual->P[i + 1], id, byteoffset, cab)) == NULL) {
                // caso retorne NULL, então o add_novo_no não fez split
                free(atual);
                return NULL;
            }
            break;
        } else if (i == 3 && id > atual->C[i] && atual->P[4] != -1) {
            // se for maior que a última chave e exista o último ponteiro, a recursão vai para o filho mais a direita do nó
            if ((retorno = ab_inserir_recur(f, atual->P[4], id, byteoffset, cab)) == NULL) {
                // caso retorne NULL, então o add_novo_no não fez split
                free(atual);
                return NULL;
            }
            break;
        }
    }

    // se retorno é NULL, então não tenho nenhum filho e sou nó filho
    if (retorno == NULL) {
        // sou um nó folha = vou adicionar em mim
        return add_novo_no(f, RRN_atual, atual, id, byteoffset, -1, -1, cab); // terei que promover alguém desse nó após inserir?
    } else {
        // tive que dar split -> preciso propagar!
        // adicionar no meu atual e ver se precisa propagar novamente
        
        int id_retorno = retorno->C;
        long int byteoffset_retorno = retorno->PR;
        int filho_esq = retorno->Pesq;
        int filho_dir = retorno->Pdir;
        // desalocar o retorno pois não vou utilizar mais e vou gerar um novo retorno
        free(retorno);

        return add_novo_no(f, RRN_atual, atual, id_retorno, byteoffset_retorno, filho_esq, filho_dir, cab);
    }
}

/*
Essa função faz a inserção de fato e vê se precisa de algum split, caso precise,
passa as informações da promoção para serem propagadas para os vértices "pais"
*/

NO_AUX *add_novo_no(FILE *f, int RRN_atual, NO_AB *atual, int id, long int byteoffset, int filho_esq, int filho_dir, CABECALHO_AB *cab) {
    NO_AUX *retorno = NULL;
    if (atual->NroChaves < 3) {
        // como tem espaço no nó, não vou precisar de split
        // tratar cada caso (em qual posição vamos inserir a chave nova?)
        if (id < atual->C[1]) {
            // caso o id a ser inserido seja menor que a primeira chave = inserir no começo
            // inserindo no começo os caras que tinham indice 1 e 2 são shiftados (ponteiros para filhos tbm)
            atual->P[4] = atual->P[3];
            atual->P[3] = atual->P[2];
            atual->P[2] = filho_dir;
            // acabei de inserir o 1o ent não tem filho no 1o ponteiro (-1)
            // de resto ocorre shift também
            atual = mudar_pos_chaves(atual, -1, 1, 2, id, byteoffset); // funcao para alterar as chaves dos filhos pos shift
        } else if (atual->C[2] == -1 || id < atual->C[2]) {
            // caso a segunda chave não exista, ou caso o id a ser inserido seja menor que a segunda chave,
            // insiro na segunda posição
            atual->P[4] = atual->P[3]; // shifto apenas o cara com id 2 para ele ser o 3o agora (shifto o ponteiro pro filho junto)
            atual->P[3] = filho_dir;
            atual = mudar_pos_chaves(atual, 1, -1, 2, id, byteoffset);
        } else {
            // caso não passe em nenhum, adiciona na 3a posição
            // nao preciso shiftar, apenas marcar o filho_dir como último ponteiro (mais a direita)
            atual->P[4] = filho_dir;
            atual = mudar_pos_chaves(atual, 1, 2, -1, id, byteoffset);
        }

        // atualizar o numero de chaves no nó
        atual->NroChaves++;

        // escrever no arquivo o nó atualizado
        ab_escrever_no(f, RRN_atual, atual);

        // não utilizarei mais o nó atual
        free(atual);
    } else {
        retorno = criar_aux_null();
        // como não tem espaço no nó, preciso dar split
        // vou inserir {id, byteoffset} na posicao adequada

        // o novo nó a ser inserido na árvore-B (nó da direita)
        NO_AB *novo_no = inicializar_no();
        novo_no->alturaNo = atual->alturaNo;
        novo_no->NroChaves = 2; // nó da direita terá 2 chaves pós split

        if (id < atual->C[1]) { // se adicionasse no atual -> novo 1 2 3 (id 1 sobe)
            // 2 e 3 viram 1 e 2 no novo nó (da direita criado)
            // o que vai retornar será a primeira chave do nó antigo
            // vou atualizar o nó esquerdo com os dados corretos, sem precisar criar um novo

            // primeira chave do novo nó
            novo_no->C[1] = atual->C[2];
            novo_no->PR[1] = atual->PR[2];
            novo_no->P[1] = atual->P[2];

            // segunda chave do novo nó
            novo_no->C[2] = atual->C[3];
            novo_no->PR[2] = atual->PR[3];
            novo_no->P[2] = atual->P[3];

            // terceiro ponteiro no novo nó
            novo_no->P[3] = atual->P[4];

            // a chave que vai ser propagada será a primeira chave do nó original
            retorno->C = atual->C[1];
            retorno->PR = atual->PR[1];

            // aqui eu passo os valores novos
            // preciso substituir, caso necessário, os ponteiros do retorno anterior
            atualizar_no_esq(atual, 1, id, byteoffset, filho_esq, filho_dir);
        } else if (id < atual->C[2]) {
            // O retorno será o novo (id, byteoffset)

            // primeira chave do novo nó
            // ponteiro do extremo esquerdo tem que ser o filho esquerdo do nó propagado anteriormente
            novo_no->C[1] = atual->C[2];
            novo_no->PR[1] = atual->PR[2];
            novo_no->P[1] = filho_dir;

            // segunda chave do novo nó
            novo_no->C[2] = atual->C[3];
            novo_no->PR[2] = atual->PR[3];
            novo_no->P[2] = atual->P[3];

            // terceiro ponteiro do novo nó
            novo_no->P[3] = atual->P[4];

            // a chave a ser propagada vai ser a chave que será inserida
            retorno->C = id;
            retorno->PR = byteoffset;

            // o ponteiro da esquerda da chave a ser propagada vai ser o ponteiro da direita
            atualizar_no_esq(atual, 1, atual->C[1], atual->PR[1], -1, filho_esq);
        } else {
            // inserindo no terceiro nó
            if (id < atual->C[3]) {
                // caso a chave a ser inserida seja menor que a última chave
                novo_no->C[1] = id;
                novo_no->PR[1] = byteoffset;
                novo_no->P[1] = filho_esq;

                novo_no->C[2] = atual->C[3];
                novo_no->PR[2] = atual->PR[3];
                novo_no->P[2] = filho_dir;

                novo_no->P[3] = atual->P[4];
            } else {
                // caso a chave a ser inserida seja maior que a última chave
                novo_no->C[1] = atual->C[3];
                novo_no->PR[1] = atual->PR[3];
                novo_no->P[1] = atual->P[3];

                novo_no->C[2] = id;
                novo_no->PR[2] = byteoffset;
                novo_no->P[2] = filho_esq;

                novo_no->P[3] = filho_dir;
            }
            // o retorno nesses dois casos será a segunda chave do nó original
            retorno->C = atual->C[2];
            retorno->PR = atual->PR[2];

            // atualizando o nó esquerdo
            // nó da esquerda terá apenas uma chave, chave 2 sobe
            // chave nova e chave 3 viram 2 chaves do nó criado
            atualizar_no_esq(atual, 1, atual->C[1], atual->PR[1], -1, -1);
        }
        // o ponteiro esquerdo será o RRN em que estamos trabalhando
        // o ponteiro para o nó criado vai ser o proxRRN
        retorno->Pesq = RRN_atual;
        retorno->Pdir = cab->proxRRN;

        // escrever no arquivo o nó atualizado
        ab_escrever_no(f, RRN_atual, atual);

        // escrever no arquivo o novo nó, e atualizar o proxRRN
        ab_escrever_no(f, cab->proxRRN, novo_no);
        cab->proxRRN++;

        // caso estejamos trabalhando na raiz, temos que criar uma nova raiz
        // nó de cima não existe pra propagarmos
        if (RRN_atual == cab->noRaiz) {

            // criar a raiz e escrever
            NO_AB *nova_raiz = inicializar_no();
            nova_raiz->C[1] = retorno->C;
            nova_raiz->PR[1] = retorno->PR;
            nova_raiz->P[1] = retorno->Pesq;
            nova_raiz->P[2] = retorno->Pdir;
            nova_raiz->alturaNo = atual->alturaNo + 1;
            nova_raiz->NroChaves = 1;

            // escrever no arquivo
            ab_escrever_no(f, cab->proxRRN, nova_raiz);
            free(nova_raiz);

            // não existe um retorno caso crie uma nova raiz
            free(retorno);
            retorno = NULL;

            // atualizar os dados do cabeçalho
            cab->noRaiz = cab->proxRRN;
            cab->proxRRN++;
        }

        // não utilizarei mais o nó atual
        free(atual);
        // não utilizarei mais o nó novo
        free(novo_no);
    }
    return retorno;
}

/*
Nessa função atualizamos os valores do id e byteoffset de cada posição
*/
// No nó atual teremos k1 k2 k3 (nessa ordem)
// Caso um deles seja -1 iremos colocar o novo valor nele ou seja o id e byteoffset passado
NO_AB *mudar_pos_chaves(NO_AB *atual, int k1, int k2, int k3, int id, int byteoffset) {
    if (k3 == -1) { // na 3a posicao queremos o valor novo
        atual->C[3] = id;
        atual->PR[3] = byteoffset;
    } else { // se nao colocamos o cara "k3" (é quem será o novo 3o valor)
        atual->C[3] = atual->C[k3];
        atual->PR[3] = atual->PR[k3];
    }
    if (k2 == -1) { // na 2a posição queremos o novo valor
        atual->C[2] = id;
        atual->PR[2] = byteoffset;
    } else { // se nao colocamos o cara "k2" (quem será o novo 2o valor)
        atual->C[2] = atual->C[k2];
        atual->PR[2] = atual->PR[k2];
    }
    if (k1 == -1) { // na 1a posicao queremos o novo valor
        atual->C[1] = id;
        atual->PR[1] = byteoffset;
    } else { // se nao colocamos o cara "k1" (quem será o novo 1o valor)
        atual->C[1] = atual->C[k1];
        atual->PR[1] = atual->PR[k1];
    }
    return atual;
}

/*
Essa função serve para atualizar o nó da esquerda com os valores pós split
O nó da esquerda ficará da seguinte forma:
(id, byteoffset) (-1, -1), (-1, -1)
e poderá ter dois ponteiros P[1] e P[2] (Pesq/Pdir) tendo apenas uma chave
*/
void atualizar_no_esq(NO_AB *atual, int k, int id, long int byteoffset, int Pesq, int Pdir) {
    atual->C[1] = id;
    atual->C[2] = atual->C[3] = -1;

    atual->PR[1] = byteoffset;
    atual->PR[2] = atual->PR[3] = -1;

    if (Pesq != -1)
        atual->P[1] = Pesq;
    if (Pdir != -1)
        atual->P[2] = Pdir;
    atual->P[3] = atual->P[4] = -1;

    atual->NroChaves = 1;
}

// função para escrever o nó no arquivo
void ab_escrever_no(FILE *f, int RRN, NO_AB *no) {
    fseek(f, (RRN + 1) * 60, SEEK_SET);
    fwrite(&no->alturaNo, sizeof(int), 1, f);
    fwrite(&no->NroChaves, sizeof(int), 1, f);
    for (int i = 1; i <= 3; i++) {
        fwrite(&no->C[i], sizeof(int), 1, f);
        fwrite(&no->PR[i], sizeof(long int), 1, f);
    }
    for (int i = 1; i <= 4; i++) {
        fwrite(&no->P[i], sizeof(int), 1, f);
    }
}

// Função para ler um nó do arquivo, sendo esse salvo na posicao 60*RRN + 60 (cabeçalho)
// Cada registro tem tamanho fixo = 60
NO_AB *ab_ler_no(FILE *f, int RRN) {
    NO_AB *atual = (NO_AB *)calloc(1, sizeof(NO_AB));
    fseek(f, (RRN + 1) * 60, SEEK_SET);

    fread(&atual->alturaNo, sizeof(int), 1, f);
    fread(&atual->NroChaves, sizeof(int), 1, f);
    for (int i = 1; i <= 3; i++) {
        fread(&atual->C[i], sizeof(int), 1, f);
        fread(&atual->PR[i], sizeof(long int), 1, f);
    }
    for (int i = 1; i <= 4; i++) {
        fread(&atual->P[i], sizeof(int), 1, f);
    }

    return atual;
}

// Função que chama a busca pelo id
// Se houver chaves, procuramos o nó caso contrário ele não estará na arvb.
// Quando a busca é pelo ID podemos utilizar buscar binária lendo apenas os nós que passamos
// ou seja apenas esses nós precisam ser passados para memória RAM
long int ab_buscar(FILE *findex, int id, CABECALHO_AB *cab) {
    if (!cab->nroChaves) {
        // nao existe nó
        return -1;
    } else {
        // buscar recursivamente
        return ab_buscar_recur(findex, cab->noRaiz, id);
    }
}

/*
Função de busca implementada de forma recursiva
*/
long int ab_buscar_recur(FILE *f, int RRN_atual, int id) {
    NO_AB *atual = ab_ler_no(f, RRN_atual);
    for (int i = 1; i <= atual->NroChaves; i++) {
        if (atual->C[i] == id) {
            long int byteoffset = atual->PR[i];
            free(atual);
            return byteoffset;
        }
        // se alguma das chaves do nó atual tem o id buscado, retornamos o byteoffset dela

        // caso contrário para que filho devemos descer?
        if (i == 1 && id < atual->C[1] && atual->P[1] != -1) {
            // caso esteja no primeiro elemento, seja menor que ele e exista ponteiro para ele
            // procura nesse filho (chama recusão pra ele)
            long int RRN_prox = atual->P[1];
            free(atual);
            return ab_buscar_recur(f, RRN_prox, id);
        } else if (id > atual->C[i] && (atual->C[i + 1] == -1 || id < atual->C[i + 1]) && atual->P[i + 1] != -1) {
            // se eu sou maior do que o atual e menor que o proximo ou proximo não existe então eu desço nesse filho
            // caso exista ponteiros para os proximos filhos, passa para o prox filho
            long int RRN_prox = atual->P[i + 1];
            free(atual);
            return ab_buscar_recur(f, RRN_prox, id);
        } else if (i == 3 && id > atual->C[i] && atual->P[4] != -1) {
            // se for maior que a última chave e exista o último ponteiro, me leva para o último ponteiro
            long int RRN_prox = atual->P[4];
            free(atual);
            return ab_buscar_recur(f, RRN_prox, id);
        }
    }
    free(atual);
    return -1; // não achei
}

// Função que imprime as informações do nó
// Sò foi utilizado para debugar
void print_no(NO_AB *no) {
    printf("Altura: %d\n", no->alturaNo);
    printf("NroChaves: %d\n", no->NroChaves);
    for (int i = 1; i <= 3; i++) {
        printf("C[%d]: %d, PR[%d]: %ld\n", i, no->C[i], i, no->PR[i]);
    }
    for (int i = 1; i <= 4; i++) {
        printf("P[%d]: %d\n", i, no->P[i]);
    }
}
