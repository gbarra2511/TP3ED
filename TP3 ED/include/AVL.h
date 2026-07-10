#ifndef AVL_H
#define AVL_H

#include <cstddef> 
#include "Vetor.h"
#include "Stats.h"

/*
 * AVL, arvore binaria de busca balanceada, generica em chave (K) e valor (V)
 * Mantem o balanceamento por altura com rotacoes a cada insercao, garantindo
 * busca em O(log n) mesmo quando as chaves chegam em ordem crescente (caso em
 * que uma BST comum degeneraria para O(n)). Requer apenas o operador < sobre K
 * No proj, indexa atributos numericos (idade, timestamp, preco, qtd),
 * mapeando cada valor para a sua posting list. Por manter as chaves ordenadas,
 * suporta consulta por faixa [min, max] de forma eficiente
 */
template <typename K, typename V>
class AVL {
private:
    struct No {
        K chave;
        V valor;
        No* esq;
        No* dir;
        int altura;
        No(const K& c) : chave(c), esq(nullptr), dir(nullptr), altura(1) {}
    };

    No* raiz;
    size_t n;
    int alturaDe(No* no) const {
        return (no == nullptr) ? 0 : no->altura;
    }

    int maior(int a, int b) const {
        return (a > b) ? a : b;
    }

    void atualizarAltura(No* no) {
        no->altura = 1 + maior(alturaDe(no->esq), alturaDe(no->dir));
    }

    int fatorBalanceamento(No* no) const {
        return (no == nullptr) ? 0 : alturaDe(no->esq) - alturaDe(no->dir);
    }

// Rotacoes simples: direita e esquerda.
    No* rotacaoDireita(No* y) {
        STAT_INC(avlRotacoes);
        No* x = y->esq;
        No* t = x->dir;
        x->dir = y;
        y->esq = t;
        atualizarAltura(y);
        atualizarAltura(x);
        return x;
    }

    No* rotacaoEsquerda(No* x) {
        STAT_INC(avlRotacoes);
        No* y = x->dir;
        No* t = y->esq;
        y->esq = x;
        x->dir = t;
        atualizarAltura(x);
        atualizarAltura(y);
        return y;
    }

    // Rebalanceia o no apos uma insercao, aplicando a rotacao adequada
    No* balancear(No* no) {
        atualizarAltura(no);
        int fb = fatorBalanceamento(no);
        if (fb > 1) {
            if (fatorBalanceamento(no->esq) < 0) {
                no->esq = rotacaoEsquerda(no->esq); // caso esquerda-direita
            }
            return rotacaoDireita(no); // caso esquerda-esquerda
        }
        if (fb < -1) {
            if (fatorBalanceamento(no->dir) > 0) {
                no->dir = rotacaoDireita(no->dir); // caso direita-esquerda
            }
            return rotacaoEsquerda(no); // caso direita-direita
        }
        return no;
    }

    //insere recursivamente
    No* inserirRec(No* no, const K& chave, V*& refValor) {
        if (no == nullptr) {
            No* novo = new No(chave);
            refValor = &novo->valor;
            n++;
            return novo;
        }
        STAT_INC(avlComparacoes);
        if (chave < no->chave) {
            no->esq = inserirRec(no->esq, chave, refValor);
        } else if (no->chave < chave) {
            no->dir = inserirRec(no->dir, chave, refValor);
        } else {
            refValor = &no->valor; // chave ja existe
            return no;
        }
        return balancear(no);
    }

    // Percurso em ordem podado, coleta valores com chave em [min, max],
    // em ordem crescente de chave, ignorando subarvores fora do intervalo.
    void coletarRec(No* no, const K& min, const K& max, Vetor<V*>& saida) const {
        if (no == nullptr) return;
        STAT_INC(avlVisitas);
        if (min < no->chave) {
            coletarRec(no->esq, min, max, saida);
        }
        if (!(no->chave < min) && !(max < no->chave)) {
            saida.adicionar(&no->valor); // min <= chave <= max
        }
        if (no->chave < max) {
            coletarRec(no->dir, min, max, saida);
        }
    }

    No* copiarRec(No* no) {
        if (no == nullptr) return nullptr;
        No* novo = new No(no->chave);
        novo->valor = no->valor;
        novo->altura = no->altura;
        novo->esq = copiarRec(no->esq);
        novo->dir = copiarRec(no->dir);
        return novo;
    }

    void destruirRec(No* no) {
        if (no == nullptr) return;
        destruirRec(no->esq);
        destruirRec(no->dir);
        delete no;
    }

public:
    AVL() : raiz(nullptr), n(0) {}

    AVL(const AVL& outro) : raiz(nullptr), n(outro.n) {
        raiz = copiarRec(outro.raiz);
    }

    AVL& operator=(const AVL& outro) {
        if (this != &outro) {
            destruirRec(raiz);
            raiz = copiarRec(outro.raiz);
            n = outro.n;
        }
        return *this;
    }

    ~AVL() { destruirRec(raiz); }

    // devolve referencia ao valor da chave, criando-o vazio se nao existir.
    V& inserir(const K& chave) {
        V* ref = nullptr;
        raiz = inserirRec(raiz, chave, ref);
        return *ref;
    }

    // busca exata somente leitura: ponteiro ao valor ou nullptr se ausente.
    const V* buscar(const K& chave) const {
        No* atual = raiz;
        while (atual != nullptr) {
            STAT_INC(avlComparacoes);
            if (chave < atual->chave) {
                atual = atual->esq;
            } else if (atual->chave < chave) {
                atual = atual->dir;
            } else {
                return &atual->valor;
            }
        }
        return nullptr;
    }

    // Coleta, em ordem crescente, ponteiros aos valores com chave em [min, max].
    void coletarFaixa(const K& min, const K& max, Vetor<V*>& saida) const {
        coletarRec(raiz, min, max, saida);
    }

    size_t tamanho() const { return n; }
};

#endif 
