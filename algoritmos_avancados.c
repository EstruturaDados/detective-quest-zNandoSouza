#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h> // Para tolower()

// --- DEFINIÇÕES GLOBAIS PARA HASH ---
#define TAMANHO_HASH 7 // Tamanho da Tabela Hash (primo para melhor distribuição)

// ====================================================================
// NÍVEL NOVO: MAPA DA MANSÃO (Árvore Binária)
// ====================================================================

// Estrutura do nó (cômodo)
typedef struct Sala {
    char nome[50];
    char pista_encontrada[100]; // String para a pista desta sala
    struct Sala *esquerda;
    struct Sala *direita;
} Sala;

// Função para criar uma nova sala (nó)
Sala* criarSala(const char *nome, const char *pista) {
    Sala *novaSala = (Sala*)malloc(sizeof(Sala));
    if (novaSala == NULL) {
        perror("Erro ao alocar memória para a sala");
        exit(EXIT_FAILURE);
    }
    strncpy(novaSala->nome, nome, 49);
    novaSala->nome[49] = '\0';
    strncpy(novaSala->pista_encontrada, pista, 99);
    novaSala->pista_encontrada[99] = '\0';
    novaSala->esquerda = NULL;
    novaSala->direita = NULL;
    return novaSala;
}

// ====================================================================
// NÍVEL AVENTUREIRO: PISTAS (Árvore de Busca - BST)
// ====================================================================

// Estrutura do nó da Árvore de Busca (Pista)
typedef struct PistaNode {
    char texto[100];
    struct PistaNode *esquerda;
    struct PistaNode *direita;
} PistaNode;

// Função para criar um novo nó de pista
PistaNode* criarPistaNode(const char *texto) {
    PistaNode *novaPista = (PistaNode*)malloc(sizeof(PistaNode));
    if (novaPista == NULL) {
        perror("Erro ao alocar memória para a pista");
        exit(EXIT_FAILURE);
    }
    strncpy(novaPista->texto, texto, 99);
    novaPista->texto[99] = '\0';
    novaPista->esquerda = NULL;
    novaPista->direita = NULL;
    return novaPista;
}

// Função para inserir uma pista na BST
PistaNode* inserirPista(PistaNode *raiz, const char *texto) {
    if (raiz == NULL) {
        return criarPistaNode(texto);
    }

    int comparacao = strcmp(texto, raiz->texto);

    if (comparacao < 0) {
        raiz->esquerda = inserirPista(raiz->esquerda, texto);
    } else if (comparacao > 0) {
        raiz->direita = inserirPista(raiz->direita, texto);
    }
    // Ignora se for igual (para não ter pistas duplicadas)

    return raiz;
}

// Função para exibir as pistas em ordem alfabética (Em Ordem)
void listarPistas(PistaNode *raiz) {
    if (raiz != NULL) {
        listarPistas(raiz->esquerda);
        printf("   - %s\n", raiz->texto);
        listarPistas(raiz->direita);
    }
}

// ====================================================================
// NÍVEL MESTRE: SUSPEITOS E ASSOCIAÇÕES (Tabela Hash)
// ====================================================================

// Estrutura para um nó de lista encadeada de Suspeitos na Hash
typedef struct Suspeito {
    char nome[50];
    int contagem_pistas;
    struct Suspeito *proximo; // Para tratamento de colisões
    
    // Lista de pistas associadas (simplificação: apenas a contagem é usada)
    // Para armazenar a lista de pistas, seria necessário outra estrutura de lista aqui.
} Suspeito;

// Tabela Hash (Array de ponteiros para a lista encadeada de Suspeitos)
Suspeito* tabelaHash[TAMANHO_HASH];

// Função de Hashing Simples: Soma dos valores ASCII e Módulo
int calcularHash(const char *nome) {
    unsigned int hash_val = 0;
    for (int i = 0; nome[i] != '\0'; i++) {
        hash_val = hash_val * 31 + tolower(nome[i]); // Multiplicador primo para espalhamento
    }
    return hash_val % TAMANHO_HASH;
}

// Função para inicializar a Tabela Hash
void inicializarHash() {
    for (int i = 0; i < TAMANHO_HASH; i++) {
        tabelaHash[i] = NULL;
    }
}

// Função para buscar ou criar um Suspeito na Tabela Hash
Suspeito* buscarOuCriarSuspeito(const char *nomeSuspeito) {
    int indice = calcularHash(nomeSuspeito);
    Suspeito *atual = tabelaHash[indice];

    // 1. Busca pelo suspeito na lista encadeada
    while (atual != NULL) {
        if (strcmp(atual->nome, nomeSuspeito) == 0) {
            return atual; // Suspeito encontrado
        }
        atual = atual->proximo;
    }

    // 2. Se não encontrado, cria um novo nó (tratamento de colisão por encadeamento)
    Suspeito *novoSuspeito = (Suspeito*)malloc(sizeof(Suspeito));
    if (novoSuspeito == NULL) {
        perror("Erro ao alocar Suspeito");
        exit(EXIT_FAILURE);
    }
    strncpy(novoSuspeito->nome, nomeSuspeito, 49);
    novoSuspeito->nome[49] = '\0';
    novoSuspeito->contagem_pistas = 0;
    
    // Insere no início da lista encadeada na posição da hash
    novoSuspeito->proximo = tabelaHash[indice];
    tabelaHash[indice] = novoSuspeito;

    return novoSuspeito;
}

// Função para associar uma pista a um Suspeito (e incrementar a contagem)
void inserirNaHash(const char *nomeSuspeito) {
    Suspeito *s = buscarOuCriarSuspeito(nomeSuspeito);
    if (s != NULL) {
        s->contagem_pistas++;
    }
}

// Função para listar todas as associações de Suspeitos
void listarAssociacoes() {
    printf("\n--- ASSOCIACÕES DE SUSPEITOS (Tabela Hash) ---\n");
    int totalSuspeitos = 0;
    for (int i = 0; i < TAMANHO_HASH; i++) {
        Suspeito *atual = tabelaHash[i];
        if (atual != NULL) {
            totalSuspeitos++;
            printf("Bucket [%d]:\n", i);
        }
        while (atual != NULL) {
            printf("  -> Suspeito: %s (Pistas Coletadas: %d)\n", atual->nome, atual->contagem_pistas);
            atual = atual->proximo;
        }
    }
    if (totalSuspeitos == 0) {
        printf("Nenhuma associação registrada ainda.\n");
    }
}

// Função para encontrar o Suspeito Mais Provável
void suspeitoMaisProvavel() {
    Suspeito *maisCitado = NULL;
    int maxPistas = -1;

    for (int i = 0; i < TAMANHO_HASH; i++) {
        Suspeito *atual = tabelaHash[i];
        while (atual != NULL) {
            if (atual->contagem_pistas > maxPistas) {
                maxPistas = atual->contagem_pistas;
                maisCitado = atual;
            }
            atual = atual->proximo;
        }
    }

    printf("\n--- RESOLUÇÃO DO MISTÉRIO ---\n");
    if (maisCitado != NULL && maxPistas > 0) {
        printf("Baseado nas %d pistas coletadas, o **SUSPEITO MAIS PROVÁVEL** é: **%s**!\n", maxPistas, maisCitado->nome);
    } else {
        printf("Ainda não há evidências suficientes para apontar um suspeito.\n");
    }
    printf("------------------------------\n");
}

// ====================================================================
// FUNÇÃO PRINCIPAL DE NAVEGAÇÃO
// ====================================================================

// Função que contém a lógica de navegação e coleta de pistas
void explorarSalas(Sala *raizMapa, PistaNode **raizPistas) {
    if (raizMapa == NULL) return;

    Sala *salaAtual = raizMapa;
    char escolha;

    printf("\n--- Início da Exploração ---\n");
    printf("Você está no: **%s**\n", salaAtual->nome);

    while (salaAtual != NULL) {
        // --- Lógica de Coleta de Pistas (Nível Aventureiro/Mestre) ---
        if (strlen(salaAtual->pista_encontrada) > 0) {
            // Verifica se a pista já foi coletada (para evitar reprocessamento)
            static PistaNode *pistas_coletadas = NULL;
            pistas_coletadas = inserirPista(pistas_coletadas, salaAtual->pista_encontrada);
            
            // Se a pista foi inserida com sucesso (assumindo que não é duplicada)
            if (pistas_coletadas) {
                 printf("🚨 Pista Encontrada! -> \"%s\"\n", salaAtual->pista_encontrada);

                // Associa a pista a um suspeito (Hardcoded para demonstração do Hash)
                const char *suspeito = "Desconhecido";
                if (strstr(salaAtual->pista_encontrada, "Relógio")) suspeito = "Mordomo";
                else if (strstr(salaAtual->pista_encontrada, "Óculos")) suspeito = "Bibliotecário";
                else if (strstr(salaAtual->pista_encontrada, "Luva")) suspeito = "Jardineiro";
                else if (strstr(salaAtual->pista_encontrada, "Carta")) suspeito = "Mordomo"; // Mais uma pista para o Mordomo!
                
                inserirNaHash(suspeito);
                printf(" -> Pista associada a: **%s**\n", suspeito);
            }
            // Limpa a pista_encontrada para não coletá-la novamente
            salaAtual->pista_encontrada[0] = '\0';
        }
        
        // Verifica se é um nó folha
        if (salaAtual->esquerda == NULL && salaAtual->direita == NULL) {
            printf("Fim do caminho. Não há mais salas para explorar a partir daqui. Pressione 's' para sair ou 'r' para revisar.\n");
        }

        printf("\nOpções: (e: Esquerda, d: Direita, r: Revisar Pistas, x: Ver Suspeitos, s: Sair): ");
        if (scanf(" %c", &escolha) != 1) {
            while (getchar() != '\n'); // Limpa buffer
            continue;
        }

        // --- Processa a Escolha ---
        if (escolha == 's' || escolha == 'S') {
            printf("\nExploração encerrada. Saindo da mansão.\n");
            break;
        } else if (escolha == 'r' || escolha == 'R') {
            printf("\n--- CADERNO DE EVIDÊNCIAS (Pistas em Ordem Alfabética) ---\n");
            listarPistas(*raizPistas);
            printf("----------------------------------------------------------\n");
            continue; // Permanece na sala atual
        } else if (escolha == 'x' || escolha == 'X') {
            listarAssociacoes();
            suspeitoMaisProvavel();
            continue; // Permanece na sala atual
        } else if (escolha == 'e' || escolha == 'E') {
            if (salaAtual->esquerda != NULL) {
                salaAtual = salaAtual->esquerda;
                printf("Você foi para a Esquerda. Nova sala: **%s**\n", salaAtual->nome);
            } else {
                printf("Não há caminho para a esquerda. Tente novamente.\n");
            }
        } else if (escolha == 'd' || escolha == 'D') {
            if (salaAtual->direita != NULL) {
                salaAtual = salaAtual->direita;
                printf("Você foi para a Direita. Nova sala: **%s**\n", salaAtual->nome);
            } else {
                printf("Não há caminho para a direita. Tente novamente.\n");
            }
        } else {
            printf("Opção inválida.\n");
       }
    }
    printf("--- Fim da Exploração ---\n");
}

// ====================================================================
// FUNÇÕES DE LIBERAÇÃO DE MEMÓRIA
// ====================================================================

// Função para liberar a memória do Mapa (Árvore Binária)
void liberarMapa(Sala *raiz) {
    if (raiz != NULL) {
        liberarMapa(raiz->esquerda);
        liberarMapa(raiz->direita);
        free(raiz);
    }
}

// Função para liberar a memória da BST de Pistas
void liberarPistas(PistaNode *raiz) {
    if (raiz != NULL) {
        liberarPistas(raiz->esquerda);
        liberarPistas(raiz->direita);
        free(raiz);
    }
}

// Função para liberar a memória da Tabela Hash
void liberarHash() {
    for (int i = 0; i < TAMANHO_HASH; i++) {
        Suspeito *atual = tabelaHash[i];
        while (atual != NULL) {
            Suspeito *temp = atual;
            atual = atual->proximo;
            free(temp);
        }
        tabelaHash[i] = NULL;
    }
}

// ====================================================================
// CONSTRUÇÃO E MAIN
// ====================================================================

// Função para construir o mapa da mansão com pistas
Sala* construirMapa() {
    // Nível 0: Raiz
    Sala *hallEntrada = criarSala("Hall de Entrada", "");

    // Nível 1
    hallEntrada->esquerda = criarSala("Cozinha", "");
    hallEntrada->direita = criarSala("Biblioteca", "Relógio de bolso quebrado");

    // Nível 2
    hallEntrada->esquerda->esquerda = criarSala("Despensa", "Caixa vazia de charutos");
    hallEntrada->esquerda->direita = criarSala("Jardim de Inverno", "Luva de jardinagem suja");

    hallEntrada->direita->esquerda = criarSala("Escritório", "Óculos deixados sobre a mesa");
    hallEntrada->direita->direita = criarSala("Sala de Jantar", "");

    // Nível 3 (Pistas importantes no final do caminho)
    hallEntrada->direita->esquerda->direita = criarSala("Quarto Principal", "Carta anônima amassada");
    hallEntrada->direita->direita->esquerda = criarSala("Varanda", "");
    
    // Caminho da Cozinha (Nível 3)
    hallEntrada->esquerda->direita->direita = criarSala("Lavanderia", ""); 

    return hallEntrada;
}

int main() {
    // Inicialização das estruturas
    Sala *mapaMansao = construirMapa();
    PistaNode *pistas = NULL;
    inicializarHash();

    printf("🕵️ Detective Quest - A Mansão dos Mistérios 🕵️\n");
    printf("Explore a mansão, colete pistas e descubra o suspeito!\n");

    // Inicia a exploração interativa
    explorarSalas(mapaMansao, &pistas);

    // Apresenta o resultado final
    suspeitoMaisProvavel();
    
    // Libera a memória alocada
    liberarMapa(mapaMansao);
    liberarPistas(pistas);
    liberarHash();
    
    return 0;
}