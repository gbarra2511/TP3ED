#include "MercadoDCC.h"
#include <iostream>
#include <string>
#include "Stats.h"

/*
 * main.cpp
 *
 * Le a entrada linha a linha, separa cada linha em tokens (campos sem espaco) e
 * delega o processamento ao sistema MercadoDCC. Toda a logica das operacoes
 * vive na classe; o main cuida apenas da leitura e da tokenizacao.
 */

// Separa a linha em tokens por espacos em branco, ignorando tabs e o '\r'
// de finais de linha no estilo Windows (robustez na correcao em Linux).
static Vetor<std::string> tokenizar(const std::string& linha) {
    Vetor<std::string> tokens;
    std::string atual;
    for (size_t i = 0; i < linha.size(); i++) {
        char c = linha[i];
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            if (!atual.empty()) {
                tokens.adicionar(atual);
                atual.clear();
            }
        } else {
            atual += c;
        }
    }
    if (!atual.empty()) {
        tokens.adicionar(atual);
    }
    return tokens;
}

int main() {
    // Acelera a E/S desacoplando dos buffers do C.
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);

    MercadoDCC mercado;
    std::string linha;
    while (std::getline(std::cin, linha)) {
        Vetor<std::string> tokens = tokenizar(linha);
        if (tokens.tamanho() == 0) continue; // ignora linhas em branco
        mercado.processar(tokens);
    }

#ifdef TP_STATS
    const char* nomesCmd[8] = {"U", "P", "R", "C", "LU", "LP", "LC", "LR"};
    std::cerr << "=== ESTATISTICAS (TP_STATS) ===\n";
    for (int i = 0; i < 8; i++) {
        if (g_stats.contagem[i] > 0) {
            std::cerr << nomesCmd[i] << " media_us "
                      << (g_stats.tempoTotal[i] / g_stats.contagem[i])
                      << " chamadas " << g_stats.contagem[i] << "\n";
        }
    }
    std::cerr << "Vetor acessos " << g_stats.vetorAcessos
              << " realocacoes " << g_stats.vetorRealocacoes
              << " copias_elem " << g_stats.vetorCopiasElem << "\n";
    std::cerr << "Hash funcao " << g_stats.hashFuncao
              << " comparacoes " << g_stats.hashComparacoes
              << " rehash " << g_stats.hashRehash << "\n";
    std::cerr << "AVL comparacoes " << g_stats.avlComparacoes
              << " rotacoes " << g_stats.avlRotacoes
              << " visitas_faixa " << g_stats.avlVisitas << "\n";
#endif

    return 0;
}
