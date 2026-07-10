#ifndef STATS_H
#define STATS_H

/*
 * Stats.h
 *
 * Instrumentacao opcional, ativada apenas quando o programa e compilado com
 * -DTP_STATS (alvo "make stats"). Sem essa macro, STAT_INC/STAT_ADD viram
 * no-ops e sao removidos pelo compilador: o binario de submissao (make all)
 * fica sem nenhum custo de medicao e com a saida padrao limpa para o VPL.
 *
 * Os contadores medem o custo algoritmico independente de E/S (comparacoes,
 * acessos, realocacoes, rotacoes), complementando a medicao de tempo feita
 * com <chrono> no despachante de comandos.
 */

#ifdef TP_STATS

struct Estatisticas {
    // Vetor
    unsigned long vetorAcessos;     // chamadas a operator[]
    unsigned long vetorRealocacoes; // dobramentos de capacidade
    unsigned long vetorCopiasElem;  // elementos copiados em realocacoes

    // Hash
    unsigned long hashFuncao;       // calculos da funcao de hash
    unsigned long hashComparacoes;  // comparacoes de chave nos baldes
    unsigned long hashRehash;       // redimensionamentos da tabela

    // AVL
    unsigned long avlComparacoes;   // comparacoes de chave (insercao/busca)
    unsigned long avlRotacoes;      // rotacoes aplicadas
    unsigned long avlVisitas;       // nos visitados em consultas por faixa

    // Tempo por comando (0=U,1=P,2=R,3=C,4=LU,5=LP,6=LC,7=LR), em microssegundos
    double tempoTotal[8];
    unsigned long contagem[8];

    Estatisticas()
        : vetorAcessos(0), vetorRealocacoes(0), vetorCopiasElem(0),
          hashFuncao(0), hashComparacoes(0), hashRehash(0),
          avlComparacoes(0), avlRotacoes(0), avlVisitas(0) {
        for (int i = 0; i < 8; i++) {
            tempoTotal[i] = 0.0;
            contagem[i] = 0;
        }
    }
};

extern Estatisticas g_stats;

#define STAT_INC(campo) (g_stats.campo++)
#define STAT_ADD(campo, n) (g_stats.campo += (n))

#else

#define STAT_INC(campo) ((void)0)
#define STAT_ADD(campo, n) ((void)0)

#endif // TP_STATS

#endif // STATS_H
