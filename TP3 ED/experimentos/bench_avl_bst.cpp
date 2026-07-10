// bench_avl_bst.cpp
//
// Compara a AVL do TP (balanceada, com rotacoes) contra uma BST simples,
// inserindo n chaves em ordem crescente (pior caso da BST) e em ordem
// aleatoria. Mede o numero de comparacoes e o tempo. A BST usa insercao
// iterativa para nao estourar a pilha no caso crescente.
//
// Compilar: g++ -std=c++11 -O2 -DTP_STATS -I../TP/include bench_avl_bst.cpp -o bench
//
// A macro TP_STATS habilita o contador g_stats.avlComparacoes dentro da AVL;
// por isso definimos aqui o objeto global g_stats que a AVL espera.

#include <iostream>
#include <cstdlib>
#include <chrono>
#include "AVL.h"

#ifdef TP_STATS
Estatisticas g_stats;
#endif

// ------- BST simples (apenas para comparacao experimental) -------
struct NoBST {
    int chave;
    NoBST* esq;
    NoBST* dir;
    NoBST(int c) : chave(c), esq(nullptr), dir(nullptr) {}
};

static unsigned long compBST = 0;

void inserirBST(NoBST*& raiz, int chave) {
    if (raiz == nullptr) {
        raiz = new NoBST(chave);
        return;
    }
    NoBST* atual = raiz;
    while (true) {
        compBST++;
        if (chave < atual->chave) {
            if (atual->esq) atual = atual->esq;
            else { atual->esq = new NoBST(chave); return; }
        } else {
            if (atual->dir) atual = atual->dir;
            else { atual->dir = new NoBST(chave); return; }
        }
    }
}

double agoraUs() {
    return std::chrono::duration<double, std::micro>(
        std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}

int main() {
    int tamanhos[] = {1000, 2000, 5000, 10000, 20000, 50000, 100000};
    int qtd = sizeof(tamanhos) / sizeof(tamanhos[0]);

    std::cout << "n,ordem,estrutura,comparacoes,tempo_us\n";

    for (int t = 0; t < qtd; t++) {
        int n = tamanhos[t];

        for (int ord = 0; ord < 2; ord++) {
            const char* ordem = (ord == 0) ? "crescente" : "aleatorio";

            // Prepara as chaves na ordem desejada.
            int* chaves = new int[n];
            for (int i = 0; i < n; i++) chaves[i] = i;
            if (ord == 1) {
                for (int i = n - 1; i > 0; i--) {
                    int j = std::rand() % (i + 1);
                    int tmp = chaves[i]; chaves[i] = chaves[j]; chaves[j] = tmp;
                }
            }

            // ----- AVL -----
            g_stats.avlComparacoes = 0;
            AVL<int, int> avl;
            double ini = agoraUs();
            for (int i = 0; i < n; i++) avl.inserir(chaves[i]) = i;
            double tAvl = agoraUs() - ini;
            std::cout << n << "," << ordem << ",avl,"
                      << g_stats.avlComparacoes << "," << tAvl << "\n";

            // ----- BST -----
            compBST = 0;
            NoBST* bst = nullptr;
            ini = agoraUs();
            for (int i = 0; i < n; i++) inserirBST(bst, chaves[i]);
            double tBst = agoraUs() - ini;
            std::cout << n << "," << ordem << ",bst,"
                      << compBST << "," << tBst << "\n";

            delete[] chaves;
        }
    }
    return 0;
}
