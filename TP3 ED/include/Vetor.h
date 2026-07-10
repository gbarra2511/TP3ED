#ifndef VETOR_H
#define VETOR_H

#include <cstddef> // size_t
#include "Stats.h"

/*
 * Vetor: arranjo dinamico generico, substituto proprio de std::vector.
 *
 * Cresce dobrando a capacidade, o que garante custo amortizado O(1) na
 * insercao ao final. Como gerencia a propria memoria com new[]/delete[],
 * implementa construtor de copia e operador de atribuicao (regra do tres)
 * para evitar copias rasas e dupla liberacao.
 *
 * E a estrutura base do projeto: armazena entidades indexadas por id,
 * as posting lists dos indices invertidos e os historicos agregados.
 */
template <typename T>
class Vetor {
private:
    T* dados;            // bloco contiguo de elementos
    size_t n;            // quantidade de elementos em uso
    size_t capacidade_;  // quantidade de posicoes alocadas

    // Garante espaco para 'minima' elementos, realocando se preciso.
    void garantirCapacidade(size_t minima) {
        if (minima <= capacidade_) return;
        size_t nova = (capacidade_ == 0) ? 1 : capacidade_ * 2;
        if (nova < minima) nova = minima;
        STAT_INC(vetorRealocacoes);
        STAT_ADD(vetorCopiasElem, n);
        T* novo = new T[nova];
        for (size_t i = 0; i < n; i++) {
            novo[i] = dados[i];
        }
        delete[] dados;
        dados = novo;
        capacidade_ = nova;
    }

public:
    Vetor() : dados(nullptr), n(0), capacidade_(0) {}

    // Copia profunda: necessaria por causa do ponteiro proprio.
    Vetor(const Vetor& outro) : dados(nullptr), n(0), capacidade_(0) {
        garantirCapacidade(outro.n);
        for (size_t i = 0; i < outro.n; i++) {
            dados[i] = outro.dados[i];
        }
        n = outro.n;
    }

    Vetor& operator=(const Vetor& outro) {
        if (this != &outro) {
            delete[] dados;
            dados = nullptr;
            n = 0;
            capacidade_ = 0;
            garantirCapacidade(outro.n);
            for (size_t i = 0; i < outro.n; i++) {
                dados[i] = outro.dados[i];
            }
            n = outro.n;
        }
        return *this;
    }

    ~Vetor() { delete[] dados; }

    // Adiciona ao final. Custo amortizado O(1).
    void adicionar(const T& valor) {
        garantirCapacidade(n + 1);
        dados[n] = valor;
        n++;
    }

    // Insere na posicao 'pos', deslocando os seguintes para a direita.
    // Usado para manter vetores em ordem crescente. Custo O(n).
    void inserir(size_t pos, const T& valor) {
        garantirCapacidade(n + 1);
        for (size_t i = n; i > pos; i--) {
            dados[i] = dados[i - 1];
        }
        dados[pos] = valor;
        n++;
    }

    // Acesso por indice (sem verificacao de limites por desempenho).
    T& operator[](size_t i) { STAT_INC(vetorAcessos); return dados[i]; }
    const T& operator[](size_t i) const { STAT_INC(vetorAcessos); return dados[i]; }

    size_t tamanho() const { return n; }
    bool vazio() const { return n == 0; }

    // Reserva capacidade antecipada para evitar realocacoes.
    void reservar(size_t c) { garantirCapacidade(c); }

    // Esvazia logicamente, mantendo a memoria ja alocada.
    void limpar() { n = 0; }
};

#endif // VETOR_H
