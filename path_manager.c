/*
 * Ficheiro:  project1.c
 * Autor:  Manuel Martins ist106454
 *
 * Descrição: Este é um programa em C que implementa um sistema de rede
 * de autocarros que permite gerenciar as paragens, linhas e ligações
 * entre elas. O programa tem vários comandos que podem ser usados para
 * adicionar, remover e exibir informações sobre as paragens de autocarros,
 * linhas e ligações.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Definicao Constantes */

#define VERDADE 1
#define FALSO 0
#define NAO_ENCONTRADO -1

/* Tamanho máximo de uma instrução */
#define MAX_INSTRUCAO 65535
/* Tamanho inicial do buffer utilizado */
#define BUFFER_CHUNK_SIZE 16

/* Definicao de Tipos de Dados */

typedef struct Paragem /*ver isto*/
{
    char *nome;
    double latitude, longitude;
    /* 1ª Carreira da Lista de carreiras que passam pela paragem. */
    struct Carreira *carreiraHead;
    struct Paragem *next;
    struct Paragem *prev;
    struct Ligacao *ligacao1;
    struct Ligacao *ligacao2;
    int numCarreiras;
} Paragem;

typedef struct Ligacao
{
    double custo, duracao;
} Ligacao;

typedef struct Carreira
{
    char *nome;
    double custoTotal;
    double duracaoTotal;
    struct Carreira *next;
    struct Carreira *prev;
    struct Paragem *paragemHead;
    struct Paragem *paragemTail;
    int numParagens;
} Carreira;

/* Funções para parsing do input */

/* Le espaços. Devolve 0 se chegou ao final de linha ou 1 caso contrário. */

int leEspacos()
{
    int c;
    while ((c = getchar()) == ' ' || c == '\t')
        ;
    if (c == '\n')
        return 0;
    ungetc(c, stdin);
    return 1;
}

/* Le um nome para a string que recebe como parametro. */

void leNome(char **s)
{
    int i = 0, c;
    int buffer_size = BUFFER_CHUNK_SIZE;             /* Tamanho buffer inicial */
    *s = (char *)malloc(buffer_size * sizeof(char)); /* Alloca memória para s */

    c = getchar();
    if (c != '"')
    {
        (*s)[i++] = c;
        while ((c = getchar()) != ' ' && c != '\t' && c != '\n')
        {
            if (i == buffer_size - 1) /* Verifica buffer está cheio */
            {
                /* Duplica tamanho de buffer */
                buffer_size *= 2;
                /* Realloc a memória */
                *s = (char *)realloc(*s, buffer_size * sizeof(char));
            }
            (*s)[i++] = c;
        }
        ungetc(c, stdin);
    }
    else
    {
        while ((c = getchar()) != '"')
        {
            if (i == buffer_size - 1) /* Verifica buffer está cheio */
            {
                /* Duplica tamanho de buffer */
                buffer_size *= 2;
                /* Realloc a memória */
                *s = (char *)realloc(*s, buffer_size * sizeof(char));
            }
            (*s)[i++] = c;
        }
    }
    (*s)[i] = '\0'; /* Termina string com '\0' */
}

void leAteFinalLinha(char **s)
{
    char c;
    int i = 0;
    int buffer_size = BUFFER_CHUNK_SIZE;
    *s = (char *)malloc(buffer_size * sizeof(char));

    while ((c = getchar()) != '\n')
    {
        if (i == buffer_size - 1)
        {
            buffer_size *= 2;
            *s = (char *)realloc(*s, buffer_size * sizeof(char));
        }
        (*s)[i++] = c;
    }
    (*s)[i] = '\0'; /* Null-terminate the string */
}

/* Funções para tratar comandos */

/* Funções para tratar as carreiras */

/* Mostra no output a carreira com nome nomeCarreira */

void mostraCarreira(Carreira *carreira)
{
    int nParagens = carreira->numParagens;

    printf("%s ", carreira->nome);

    if (nParagens > 0)
    {
        printf("%s %s ", carreira->paragemHead->nome, carreira->paragemTail->nome);
        printf("%d ", nParagens);
        printf("%.2f ", carreira->custoTotal);
        printf("%.2f\n", carreira->duracaoTotal);
    }
    else
        printf("%d %.2f %.2f\n", 0, 0.0, 0.0);
}

/* Mostra as ligações da carreira com indice i. */

void mostraLigacoesCarreira(Carreira *carreira, int inverso)
{
    Paragem *head = carreira->paragemHead;
    Paragem *tail = carreira->paragemTail;
    Paragem *current;

    if (head == NULL)
        return;

    if (inverso == FALSO)
    {
        printf("%s", head->nome);
        current = head->next;
        while (current != NULL)
        {
            printf(", %s", current->nome);
            current = current->next;
        }
        printf("\n");
    }
    else
    {
        printf("%s", tail->nome);
        current = tail->prev;
        /* Quando chega à primeira paragem da carreira*/
        while (current != NULL)
        {
            printf(", %s", current->nome);
            current = current->prev;
        }
        printf("\n");
    }
}

/* Mostra todas as carreiras. */

void listaCarreiras(Carreira *head)
{
    Carreira *current = head;

    while (current != NULL)
    {
        mostraCarreira(current);
        current = current->next;
    }
}

/* Procura uma carreira por nome.
   Devolve o a carreira ou NULL se não existe. */

Carreira *encontraCarreira(Carreira *head, char *nomeCarreira)
{
    Carreira *current = head;

    while (current != NULL)
    {
        if (strcmp(current->nome, nomeCarreira) == 0)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

/* Procura se uma paragem existe numa carreira.
   Se existir, devolve 1. Caso contrário, devolve NAO_ENCONTRADO. */

int encontraParagemCarreira(Carreira *carreira, Paragem *paragem)
{
    Paragem *current = carreira->paragemHead;

    while (current != NULL)
    {
        if (strcmp(current->nome, paragem->nome) == 0)
            return 1;
        current = current->next;
    }
    return NAO_ENCONTRADO;
}

/* Cria uma carreira nova. */

void criaCarreira(Carreira **headRef, char *nomeCarreira)
{
    Carreira *newCarreira = (Carreira *)malloc(sizeof(Carreira));
    newCarreira->nome = (char *)malloc(strlen(nomeCarreira) + 1);
    if (newCarreira->nome) /* Verifica se a memória alocada foi bem sucedida */
    {
        strcpy(newCarreira->nome, nomeCarreira);
    }
    newCarreira->custoTotal = 0.0;
    newCarreira->duracaoTotal = 0.0;
    newCarreira->numParagens = 0;
    newCarreira->paragemHead = NULL;
    newCarreira->paragemTail = NULL;
    newCarreira->next = NULL;

    /* Se a lista está vazia */
    if (*headRef == NULL)
    {
        *headRef = newCarreira;
    }
    else
    {
        /* Percorre a lista até encontrar a última carreira */
        Carreira *current = *headRef;
        while (current->next != NULL)
        {
            current = current->next;
        }
        /* adiciona a carreira ao fim da lista */
        current->next = newCarreira;
        newCarreira->prev = current;
    }
}

/* Cria uma cópia de uma carreira só com o nome. */

Carreira *carreiraCopiaNome(const Carreira *c)
{
    Carreira *newCarreira = (Carreira *)malloc(sizeof(Carreira));
    if (newCarreira == NULL)
    {
        return NULL;
    }

    newCarreira->nome = (char *)malloc(strlen(c->nome) + 1);
    if (newCarreira->nome == NULL)
    {
        free(newCarreira);
        return NULL;
    }

    strcpy(newCarreira->nome, c->nome);
    newCarreira->numParagens = 0;
    newCarreira->paragemHead = NULL;
    newCarreira->paragemTail = NULL;
    newCarreira->next = NULL;
    newCarreira->custoTotal = 0;
    newCarreira->duracaoTotal = 0;

    return newCarreira;
}

/* Verifica se a string é um prefixo de tamanho pelo menos 3 da
   palavra inverso. */

int verificaInversoOk(char *s)
{
    char inv[] = "inverso";
    int size = strlen(s), i;

    if (size < 3 || size > 7)
        return FALSO;
    for (i = 0; i < size; i++)
        if (inv[i] != s[i])
            return FALSO;
    return VERDADE;
}

/* Função para tratar do comando 'c'. */

void carreiras(Carreira **headRef)
{
    char *s;
    int i, len, fimLinha = leEspacos();
    Carreira *foundCarreira;
    char *buffer;

    if (!fimLinha)
    {
        listaCarreiras(*headRef);
        return;
    }

    leNome(&s);

    /* Exercício 1 do Teste Prático */

    len = strlen(s);
    for (i = 0; i < len; i++)
    {
        if (s[i] < 'A' || s[i] > 'Z')
        {
            printf("invalid line name. \n");
            return;
        }
    }

    fimLinha = leEspacos();
    foundCarreira = encontraCarreira(*headRef, s);
    if (!fimLinha)
    {
        if (foundCarreira == NULL)
            criaCarreira(headRef, s);
        else
            mostraLigacoesCarreira(foundCarreira, FALSO);
    }
    else
    {
        leNome(&buffer);
        if (verificaInversoOk(buffer))
            mostraLigacoesCarreira(foundCarreira, VERDADE);
        else
            printf("incorrect sort option.\n");
        leAteFinalLinha(&buffer);
        free(buffer);
    }
    free(s);
}

/* Funções para tratar as paragens */

/* Mostra uma paragem. */

void mostraParagem(Paragem *p)
{
    int len = strlen(p->nome);

    /* Exercício 2 do Teste Prático */
    if (len > 10)
        printf("%s: %16.12f %16.12f %d\n", p->nome, p->latitude,
               p->longitude, p->numCarreiras);
}

/* Mostra todas as paragens. */

void listaParagens(Paragem *head)
{
    Paragem *current = head;

    while (current != NULL)
    {
        mostraParagem(current);
        current = current->next;
    }
}

/* Verifica se existe uma paragem com um determinado nome.
   Se existir devolve a paragem. Caso contrário, devolve NULL. */

Paragem *encontraParagem(Paragem *head, char *nomeParagem)
{
    Paragem *current = head;

    while (current != NULL)
    {
        if (strcmp(current->nome, nomeParagem) == 0)
        {
            return current;
        }
        current = current->next;
    }

    return NULL;
}

/* Cria uma nova paragem. */

void criaParagem(Paragem **headRef, char *nomeParagem, double latitude, double longitude)
{
    Paragem *newParagem = (Paragem *)malloc(sizeof(Paragem));
    newParagem->nome = (char *)malloc(strlen(nomeParagem) + 1);
    if (newParagem->nome)
    {
        strcpy(newParagem->nome, nomeParagem);
    }
    newParagem->latitude = latitude;
    newParagem->longitude = longitude;
    newParagem->next = NULL;
    newParagem->prev = NULL;
    newParagem->carreiraHead = NULL;
    newParagem->ligacao1 = NULL;
    newParagem->ligacao2 = NULL;
    newParagem->numCarreiras = 0;

    /* Se a lista estiver vazia, faz a nova paragem a cabeça da lista */
    if (*headRef == NULL)
    {
        *headRef = newParagem;
    }
    else
    {
        /* Percorre a lista de paragens até encontrar a última */
        Paragem *current = *headRef;
        while (current->next != NULL)
        {
            current = current->next;
        }
        /* Adiciona a paragem no final da lista */
        current->next = newParagem;
        newParagem->prev = current;
    }
}

/* Cria uma cópia de paragem só com o nome e coordenadas */

Paragem *paragemCopiaNome(const Paragem *p)
{
    Paragem *pcopy = (Paragem *)malloc(sizeof(Paragem));
    if (pcopy == NULL)
    {
        return NULL;
    }

    pcopy->nome = (char *)malloc(strlen(p->nome) + 1);
    if (pcopy->nome == NULL)
    {
        free(pcopy);
        return NULL;
    }

    strcpy(pcopy->nome, p->nome);
    pcopy->carreiraHead = NULL;
    pcopy->latitude = p->latitude;
    pcopy->longitude = p->longitude;
    pcopy->ligacao1 = NULL;
    pcopy->ligacao2 = NULL;
    pcopy->next = NULL;
    pcopy->prev = NULL;
    pcopy->numCarreiras = p->numCarreiras;

    return pcopy;
}

/* Função para tratar o comando 'p'. */

void paragens(Paragem **headRef)
{
    char *s;
    int fimLinha = leEspacos();
    Paragem *foundParagem;
    char *buffer;

    if (!fimLinha)
    {
        listaParagens(*headRef);
        return;
    }
    leNome(&s);
    fimLinha = leEspacos();
    foundParagem = encontraParagem(*headRef, s);
    if (!fimLinha)
    {
        if (foundParagem == NULL)
            printf("%s: no such stop.\n", s);
        else
            printf("%16.12f %16.12f\n", foundParagem->latitude, foundParagem->longitude);
    }
    else
    {
        double latitude, longitude;
        scanf("%lf%lf", &latitude, &longitude);
        if (foundParagem == NULL)
            criaParagem(headRef, s, latitude, longitude);
        else
            printf("%s: stop already exists.\n", s);
        leAteFinalLinha(&buffer);
        free(buffer);
    }
    free(s);
}

/* Acrescenta uma carreira à lista de interseções da paragem*/

void adicionaCarreiraParagem(Paragem *p, Carreira *c)
{
    Carreira *copy = carreiraCopiaNome(c);
    Carreira *aux = p->carreiraHead;

    if (copy == NULL)
        return;

    if (aux == NULL)
    {
        p->carreiraHead = copy;
        p->numCarreiras++;
    }
    else
    {
        while (aux->next != NULL)
        {
            aux = aux->next;
        }
        aux->next = copy;
        copy->prev = aux;
        p->numCarreiras++;
    }
}

/* Acrescenta uma ligação numa carreira vazia */

void primeiraLigacao(Carreira *c, Paragem *p1, Paragem *p2, double custo, double duracao)
{
    Ligacao *ligacao1, *ligacao2, *ligacao3;
    Paragem *p1copy = paragemCopiaNome(p1);
    Paragem *p2copy = paragemCopiaNome(p2);
    ligacao1 = (Ligacao *)malloc(sizeof(Ligacao));
    ligacao2 = (Ligacao *)malloc(sizeof(Ligacao));
    ligacao3 = (Ligacao *)malloc(sizeof(Ligacao));

    /* ligacao1 corresponde à ligação entre p1 e a paragem que pode vir antes*/
    ligacao1->custo = 0;
    ligacao1->duracao = 0;
    /* ligacao2 corresponde à ligação entre p1 e p2 */
    ligacao2->custo = custo;
    ligacao2->duracao = duracao;
    /* ligacao3 corresponde à ligação entre p2 e a paragem que pode vir depois */
    ligacao3->custo = 0;
    ligacao3->duracao = 0;

    c->paragemHead = p1copy;
    c->paragemTail = p2copy;
    p1copy->ligacao1 = ligacao1;
    p1copy->ligacao2 = ligacao2;
    p1copy->next = p2copy;
    p2copy->prev = p1copy;
    p2copy->next = NULL;
    p2copy->ligacao1 = ligacao2;
    p2copy->ligacao2 = ligacao3;
    c->numParagens = 2;
    c->custoTotal = custo;
    c->duracaoTotal = duracao;

    /* Atualizar as listas de carreiras de p1 e p2 */
    if (strcmp(p1->nome, p2->nome) == 0)
    {
        adicionaCarreiraParagem(p1, c);
    }
    else
    {
        adicionaCarreiraParagem(p1, c);
        adicionaCarreiraParagem(p2, c);
    }
}

/* Acrescenta uma nova ligação no início de uma carreira. */

void acrescentaLigacaoInicio(Carreira *c, Paragem *p, double custo, double duracao)
{
    Ligacao *ligacao1, *ligacao2;
    Paragem *pcopy = paragemCopiaNome(p);
    ligacao1 = (Ligacao *)malloc(sizeof(Ligacao));
    ligacao2 = (Ligacao *)malloc(sizeof(Ligacao));

    /* ligacao1 corresponde à ligação entre p1 e a paragem que pode vir antes*/
    ligacao1->custo = 0;
    ligacao1->duracao = 0;
    /* ligacao2 corresponde à ligação entre p1 e a próxima paragem*/
    ligacao2->custo = custo;
    ligacao2->duracao = duracao;

    pcopy->ligacao1 = ligacao1;
    pcopy->ligacao2 = ligacao2;

    c->paragemHead->ligacao1 = ligacao2;
    pcopy->next = c->paragemHead;
    c->paragemHead->prev = pcopy;
    c->paragemHead = pcopy;

    c->numParagens++;
    c->custoTotal += custo;
    c->duracaoTotal += duracao;
}

/* Acrescenta uma nova ligação no fim de uma carreira. */

void acrescentaLigacaoFim(Carreira *c, Paragem *p, double custo, double duracao)
{
    Ligacao *ligacao1, *ligacao2;
    Paragem *pcopy = paragemCopiaNome(p);
    ligacao1 = (Ligacao *)malloc(sizeof(Ligacao));
    ligacao2 = (Ligacao *)malloc(sizeof(Ligacao));

    /* ligacao1 corresponde à ligação entre p1 e a paragem que pode vir antes*/
    ligacao1->custo = custo;
    ligacao1->duracao = duracao;
    /* ligacao2 corresponde à ligação entre p1 e a próxima paragem*/
    ligacao2->custo = 0;
    ligacao2->duracao = 0;

    pcopy->ligacao1 = ligacao1;
    pcopy->ligacao2 = ligacao2;

    c->paragemTail->ligacao2 = ligacao1;
    c->paragemTail->next = pcopy;
    pcopy->prev = c->paragemTail;
    c->paragemTail = pcopy;

    c->numParagens++;
    c->custoTotal += custo;
    c->duracaoTotal += duracao;
}

/* Adiciona uma nova ligação a uma carreira, quando ela nao é vazia. */

void adicionaLigacao(Carreira *carreira, Paragem *origem, Paragem *destino,
                     double custo, double duracao)
{
    char *carreiraHead = carreira->paragemHead->nome;
    char *carreiraTail = carreira->paragemTail->nome;
    char *origemNome = origem->nome;
    char *destinoNome = destino->nome;

    if (strcmp(carreiraTail, origemNome) == 0)
    {
        if (encontraParagemCarreira(carreira, destino) == NAO_ENCONTRADO)
            adicionaCarreiraParagem(destino, carreira);
        acrescentaLigacaoFim(carreira, destino, custo, duracao);
    }
    else if (strcmp(carreiraHead, destinoNome) == 0)
    {
        if (encontraParagemCarreira(carreira, origem) == NAO_ENCONTRADO)
            adicionaCarreiraParagem(origem, carreira);
        acrescentaLigacaoInicio(carreira, origem, custo, duracao);
    }
    else
        printf("link cannot be associated with bus line.\n");
}

/* Le nome de carreira e paragens de uma ligacao. */

void leNomesComando(char **nomeCarreira, char **nomeOrigem, char **nomeDestino)
{
    if (leEspacos())
    {
        leNome(nomeCarreira);
    }

    if (leEspacos())
    {
        leNome(nomeOrigem);
    }

    if (leEspacos())
    {
        leNome(nomeDestino);
    }
}

/* Função para tratar o comando 'l'. */

void ligacoes(Carreira **headrefCarreira, Paragem **headrefParagem)
{
    char *nomeCarreira, *nomeOrigem, *nomeDestino;
    double custo, duracao;
    char *buffer;
    Carreira *foundCarreira;
    Paragem *foundParagemOrigem, *foundParagemDestino;

    leNomesComando(&nomeCarreira, &nomeOrigem, &nomeDestino);
    scanf("%lf%lf", &custo, &duracao);
    leAteFinalLinha(&buffer);

    foundCarreira = encontraCarreira(*headrefCarreira, nomeCarreira);
    if (foundCarreira == NULL)
        printf("%s: no such line.\n", nomeCarreira);
    else
    {
        foundParagemOrigem = encontraParagem(*headrefParagem, nomeOrigem);
        if (foundParagemOrigem == NULL)
            printf("%s: no such stop.\n", nomeOrigem);
        else
        {
            foundParagemDestino = encontraParagem(*headrefParagem, nomeDestino);
            if (foundParagemDestino == NULL)
                printf("%s: no such stop.\n", nomeDestino);
            else if (custo < 0.0 || duracao < 0.0)
                printf("negative cost or duration.\n");
            else
            {
                if (foundCarreira->numParagens == 0)
                    primeiraLigacao(foundCarreira, foundParagemOrigem,
                                    foundParagemDestino, custo, duracao);
                else
                    adicionaLigacao(foundCarreira, foundParagemOrigem,
                                    foundParagemDestino, custo, duracao);
            }
        }
    }
    free(nomeCarreira);
    free(nomeOrigem);
    free(nomeDestino);
    free(buffer);
}

/* Função para ordenar uma lista por ordem alfabética */

void sortLinkedList(Paragem *paragem)
{
    Carreira *current = paragem->carreiraHead;
    Carreira *index = NULL;
    char *tempNome;

    while (current != NULL)
    {
        index = current->next;
        while (index != NULL)
        {
            if (strcmp(current->nome, index->nome) > 0)
            {
                tempNome = current->nome;
                current->nome = index->nome;
                index->nome = tempNome;
            }
            index = index->next;
        }
        current = current->next;
    }
}

/* Função que mostra as carreiras de uma paragem */

void mostraCarreirasParagem(Paragem *paragem)
{
    Carreira *current = paragem->carreiraHead;
    printf("%s %d: %s", paragem->nome, paragem->numCarreiras, current->nome);
    current = current->next;
    while (current != NULL)
    {
        printf(" %s", current->nome);
        current = current->next;
    }
    printf("\n");
}

/* Função para tratar o comando 'i'. */

void intersecoes(Paragem **headRef)
{
    Paragem *current = *headRef;
    char *buffer;

    leAteFinalLinha(&buffer);

    while (current != NULL)
    {
        if (current->numCarreiras >= 2)
        {
            sortLinkedList(current);
            mostraCarreirasParagem(current);
        }
        current = current->next;
    }
    free(buffer);
}

/* Função que elimina a carreira da lista de carreiras de uma paragem */

void eliminaCarreiraDeParagem(Paragem *paragem, char *nomeCarreira)
{
    Carreira *current = paragem->carreiraHead;
    int deleted = 0;

    while (current != NULL)
    {
        if (strcmp(current->nome, nomeCarreira) == 0)
        {
            /* Só executa paragem->numCarreiras-- uma vez */
            if (!deleted)
            {
                paragem->numCarreiras--;
                deleted = 1;
            }

            /* Se eliminar a primeira carreira da lista carreiras de paragem */
            if (current->prev == NULL)
            {

                /* Se a lista de carreiras tiver 2 ou mais carreiras */
                if (current->next != NULL)
                    current->next->prev = NULL;
                free(current);
            }
            /* Se eliminar uma carreira no meio */
            else if (current->next != NULL)
            {
                current->prev->next = current->next;
                current->next->prev = current->prev;
                free(current);
            }
            /* Se eliminar a ultima carreira da lista */
            else
            {
                current->prev->next = NULL;
                free(current);
            }
        }
        current = current->next;
    }
}

/* Função que elimina a carreira da lista de carreiras de todas as paragens */

void eliminaCarreiraTodasParagens(Paragem **headRef, char *nomeCarreira)
{
    Paragem *current = *headRef;

    while (current != NULL)
    {
        eliminaCarreiraDeParagem(current, nomeCarreira);
        current = current->next;
    }
}

/* Função que elimina uma carreira da lista de Carreiras */

void eliminaCarreiraDaLista(Carreira *carreira)
{

    Paragem *currentTail = carreira->paragemTail;
    Paragem *prevNode;

    while (currentTail != NULL)
    {
        prevNode = currentTail->prev;
        free(currentTail);
        currentTail = prevNode;
    }

    free(carreira);
}

/* Função que elimina todas as carreiras de nome nomeCarreira
 e as suas paragens da lista de carreiras */

void eliminaTodasCarreiraDaLista(Carreira **headRef, char *nomeCarreira)
{
    Carreira *current = *headRef;

    while (current != NULL)
    {
        if (strcmp(current->nome, nomeCarreira) == 0)
        {
            /* Se eliminar a primeira carreira da lista carreiras */
            if (current->prev == NULL)
            {
                *headRef = current->next;
                /* Se a lista de carreiras tiver 2 ou mais carreiras */
                if (current->next != NULL)
                    current->next->prev = NULL;
                eliminaCarreiraDaLista(current);
            }
            /* Se eliminar uma carreira no meio */
            else if (current->next != NULL)
            {
                current->prev->next = current->next;
                current->next->prev = current->prev;
                eliminaCarreiraDaLista(current);
            }
            /* Se eliminar a ultima carreira da lista */
            else
            {
                current->prev->next = NULL;
                eliminaCarreiraDaLista(current);
            }
        }
        current = current->next;
    }
}

/* Função que trata do comando r */

void comando_r(Carreira **headCarreira, Paragem **headParagem)
{
    char *s;

    leEspacos();
    leNome(&s);
    if (encontraCarreira(*headCarreira, s) == NULL)
        printf("%s: no such line.\n", s);
    eliminaTodasCarreiraDaLista(headCarreira, s);
    eliminaCarreiraTodasParagens(headParagem, s);
    free(s);
}

/* Função que elimina uma paragem de nome nomeParagem,
 da lista de paragens de uma carreira */

void eliminaParagemDeCarreira(Paragem **headRef, Carreira *carreira, char *nomeParagem)
{
    Paragem *current = carreira->paragemHead;
    /* paragem é utilizada no fim para atualizar o seu número de carreiras */
    Paragem *paragem;

    while (current != NULL)
    {
        if (strcmp(current->nome, nomeParagem) == 0)
        {
            current->numCarreiras -= 1;
            carreira->numParagens -= 1;

            /* Se eliminar a primeira paragem da lista paragens de carreira */
            if (current->prev == NULL)
            {
                /* Subtrair o custo e duração da ligação */
                carreira->custoTotal -= current->ligacao2->custo;
                carreira->duracaoTotal -= current->ligacao2->duracao;
                carreira->paragemHead = current->next;
                /* Se a lista de paragens tiver 1 paragem */
                if (current->next == NULL)
                    carreira->paragemTail = NULL;
                /* Se a lista de paragens tiver 2 ou mais paragens */
                else
                {
                    current->next->ligacao1->custo = 0;
                    current->next->ligacao1->duracao = 0;
                    current->next->prev = NULL;
                }
                free(current);
            }
            /* Se eliminar uma paragem no meio */
            else if (current->next != NULL)
            {
                /* O custo e duracao das ligações retiradas é somado*/
                current->prev->ligacao2->custo += current->ligacao2->custo;
                current->prev->ligacao2->duracao += current->ligacao2->duracao;
                current->next->ligacao1 = current->ligacao1;
                current->prev->next = current->next;
                current->next->prev = current->prev;
                free(current);
            }
            /* Se eliminar a ultima paragem da lista */
            else
            {
                /* Subtrair o custo e duração da ligação */
                carreira->custoTotal -= current->ligacao1->custo;
                carreira->duracaoTotal -= current->ligacao1->duracao;
                current->prev->next = NULL;
                carreira->paragemTail = current->prev;
                free(current);
            }
        }
        current = current->next;
    }

    /* Se a carreira ficar só com uma paragem, tem de ficar vazia */
    if (carreira->numParagens == 1)
    {
        paragem = encontraParagem(*headRef, carreira->paragemHead->nome);
        paragem->numCarreiras -= 1;
        carreira->numParagens -= 1;
        carreira->paragemHead = NULL;
        free(carreira->paragemHead);
    }
}

/* Função que elimina a paragem da lista de paragens de todas as careiras */

void eliminaParagemTodasCarreiras(Paragem **headParagem, Carreira **headCarreira,
                                  char *nomeParagem)
{
    Carreira *current = *headCarreira;

    while (current != NULL)
    {
        eliminaParagemDeCarreira(headParagem, current, nomeParagem);
        current = current->next;
    }
}

/* Função que elimina uma paragem da lista de Paragens */

void eliminaParagemDaLista(Paragem *paragem)
{

    Carreira *current = paragem->carreiraHead;
    Carreira *prevNode;

    while (current != NULL)
        current = current->next;

    while (current != NULL)
    {
        prevNode = current->prev;
        free(current);
        current = prevNode;
    }
    free(paragem);
}

/* Função que elimina todas as paragens de nome nomePargem
  e as suas carreiras da lista de Paragens */

void eliminaTodasParagensDaLista(Paragem **headRef, char *nomeCarreira)
{
    Paragem *current = *headRef;

    while (current != NULL)
    {
        if (strcmp(current->nome, nomeCarreira) == 0)
        {
            /* Se eliminar a primeira carreira da lista carreiras */
            if (current->prev == NULL)
            {
                *headRef = current->next;
                /* Se a lista de carreiras tiver 2 ou mais carreiras */
                if (current->next != NULL)
                    current->next->prev = NULL;
                eliminaParagemDaLista(current);
            }
            /* Se eliminar uma carreira no meio */
            else if (current->next != NULL)
            {
                current->prev->next = current->next;
                current->next->prev = current->prev;
                eliminaParagemDaLista(current);
            }
            /* Se eliminar a ultima carreira da lista */
            else
            {
                current->prev->next = NULL;
                eliminaParagemDaLista(current);
            }
        }
        current = current->next;
    }
}

/* Função que trata do comando e */

void comando_e(Carreira **headCarreira, Paragem **headParagem)
{
    char *s;

    leEspacos();
    leNome(&s);
    if (encontraParagem(*headParagem, s) == NULL)
        printf("%s: no such stop.\n", s);
    eliminaTodasParagensDaLista(headParagem, s);
    eliminaParagemTodasCarreiras(headParagem, headCarreira, s);
    free(s);
}

/* Função que liberta todas as paragens */

void freeParagens(Paragem **paragemHead)
{
    Paragem *current = *paragemHead;
    Paragem *aux;

    while (current != NULL)
    {
        aux = current->next;
        eliminaParagemDaLista(current);
        current = aux;
    }
    *paragemHead = NULL;
}

/* Função que liberta todas as carreiras */

void freeCarreiras(Carreira **carreiraHead)
{
    Carreira *current = *carreiraHead;
    Carreira *aux;

    while (current != NULL)
    {
        aux = current->next;
        eliminaCarreiraDaLista(current);
        current = aux;
    }
    *carreiraHead = NULL;
}

/* Exercício 3 do Teste Prático */
void comando_s(Paragem **headRef)
{
    char *s;
    int fimLinha = leEspacos();
    Paragem *foundParagem;
    Carreira *current;

    leNome(&s);
    fimLinha = leEspacos();
    foundParagem = encontraParagem(*headRef, s);
    current = foundParagem->carreiraHead;
    if (!fimLinha)
    {
        if (foundParagem == NULL)
            printf("%s: no such stop.\n", s);
        else
        {
            printf("%s", s);
            while (current != NULL)
            {
                printf(" %s", current->nome);
                current = current->next;
            }
        }
    }

    free(s);
}

/* Função MAIN */

int main()
{
    int c;
    Paragem *headParagem = NULL;   /* cria um ponteiro para a lista de paragens */
    Carreira *headCarreira = NULL; /* cria um ponteiro para a lista de carreiras */
    do
    {
        c = getchar();
        switch (c)
        {
        case 'c':
            carreiras(&headCarreira);
            break;
        case 'p':
            paragens(&headParagem);
            break;
        case 'l':
            ligacoes(&headCarreira, &headParagem);
            break;
        case 'i':
            intersecoes(&headParagem);
            break;
        case 's':
            comando_s(&headParagem);
            break;
        case 'r':
            comando_r(&headCarreira, &headParagem);
            break;
        case 'e':
            comando_e(&headCarreira, &headParagem);
            break;
        case 'a':
            freeCarreiras(&headCarreira);
            freeParagens(&headParagem);
            break;
        case 'q':
            freeCarreiras(&headCarreira);
            freeParagens(&headParagem);
            break;
        default:
            /* Ignorar linhas em branco */
            if (c == ' ' || c == '\t' || c == '\n')
                break;
        }
    } while (c != 'q');
    return 0;
}
