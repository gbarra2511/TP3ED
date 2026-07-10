#ifndef HASH_H
#define HASH_H

#include <cstddef> 
#include <string>
#include "Stats.h"

/*
 * Hash: tabela hash generica com chave string e encadeamento separado.
 *
 * Cada balde e uma lista encadeada de pares (chave, valor). Colisoes sao
 * resolvidas inserindo o novo no no inicio do bucket Quando o fator de
 * carga ultrapassa o limite, a tabela e redimensionada (rehashing) para manter a busca exata em O(1) amortizado
 * No projeto, mapeia valores textuais (cidade, marca, categoria, ...) para
 * a posting list de ids que possuem aquele valor. O valor V e tipicamente um Vetor<int>
 */
template <typename V>
class Hash {
private:
    struct No {
        std::string chave;
        V valor;
        No* prox;
        No(const std::string& c) : chave(c), prox(nullptr) {}
    };

    No** bucket;       // arranjo de ponteiros para o inicio de cada bucket
    size_t numBuckets;  // quantidade de buckets
    size_t n;          // quantidade de pares armazenados

    // Funcao djb2 sobre a string, reduzida ao numero de bucket informado
    size_t hashCom(const std::string& s, size_t mod) const {
        //implementacao padrao
        STAT_INC(hashFuncao);
        unsigned long h = 5381;

        for (size_t i = 0; i < s.size(); i++) {
            h = ((h << 5) + h) + (unsigned char) s[i]; 
        }
        return (size_t)(h % mod);
    }

    // Move os nos existentes para um novo arranjo maior de bucket
    void redimensionar(size_t novoNum) {
        STAT_INC(hashRehash);
        No** novos = new No*[novoNum];
        for (size_t i = 0; i < novoNum; i++) {
            novos[i] = nullptr;
        }
        for (size_t i = 0; i < numBuckets; i++) {
            No* atual = bucket[i];
            while (atual != nullptr) {
                No* prox = atual->prox;
                size_t idx = hashCom(atual->chave, novoNum);
                atual->prox = novos[idx];
                novos[idx] = atual;
                atual = prox;
            }
        }
        delete[] bucket;
        bucket = novos;
        numBuckets = novoNum;
    }

    // Libera todos os nos de todos os bucket.
    void limparTudo() {
        for (size_t i = 0; i < numBuckets; i++) {
            No* atual = bucket[i];
            while (atual != nullptr) {
                No* prox = atual->prox;
                delete atual;
                atual = prox;
            }
            bucket[i] = nullptr;
        }
        n = 0;
    }

public:
    Hash(size_t baldesIniciais = 1024) : numBuckets(baldesIniciais), n(0) {
        bucket = new No*[numBuckets];
        for (size_t i = 0; i < numBuckets; i++) {
            bucket[i] = nullptr;
        }
    }

    Hash(const Hash& outro) : numBuckets(outro.numBuckets), n(0) {
        bucket = new No*[numBuckets];
        for (size_t i = 0; i < numBuckets; i++) {
            bucket[i] = nullptr;
        }
        // Reinsere cada par do outro (copia profunda dos valores).
        for (size_t i = 0; i < outro.numBuckets; i++) {
            No* atual = outro.bucket[i];
            while (atual != nullptr) {
                inserir(atual->chave) = atual->valor;
                atual = atual->prox;
            }
        }
    }

    Hash& operator=(const Hash& outro) {
        if (this != &outro) {
            limparTudo();
            for (size_t i = 0; i < outro.numBuckets; i++) {
                No* atual = outro.bucket[i];
                while (atual != nullptr) {
                    inserir(atual->chave) = atual->valor;
                    atual = atual->prox;
                }
            }
        }
        return *this;
    }

    ~Hash() {
        limparTudo();
        delete[] bucket;
    }

    // Devolve referencia ao valor da chave, criando-o vazio se nao existir.
    // E o ponto de entrada para construir o indice (inserir ids na posting list).
    V& inserir(const std::string& chave) {
        size_t idx = hashCom(chave, numBuckets);
        for (No* atual = bucket[idx]; atual != nullptr; atual = atual->prox) {
            STAT_INC(hashComparacoes);
            if (atual->chave == chave) {
                return atual->valor;
            }
        }
        // Nao existe: cria no inicio do balde.
        No* novo = new No(chave);
        novo->prox = bucket[idx];
        bucket[idx] = novo;
        n++;
        // Redimensiona se o fator de carga passar de ~0.75.
        if (n > (numBuckets * 3) / 4) {
            redimensionar(numBuckets * 2);
            // Apos o rehash, reencontra o no recem-criado.
            idx = hashCom(chave, numBuckets);
            for (No* atual = bucket[idx]; atual != nullptr; atual = atual->prox) {
                STAT_INC(hashComparacoes);
                if (atual->chave == chave) {
                    return atual->valor;
                }
            }
        }
        return novo->valor;
    }

    // Busca somente leitura: devolve ponteiro ao valor ou nullptr se ausente.
    const V* buscar(const std::string& chave) const {
        size_t idx = hashCom(chave, numBuckets);
        for (No* atual = bucket[idx]; atual != nullptr; atual = atual->prox) {
            STAT_INC(hashComparacoes);
            if (atual->chave == chave) {
                return &atual->valor;
            }
        }
        return nullptr;
    }

    size_t tamanho() const { return n; }
};

#endif // HASH_H
