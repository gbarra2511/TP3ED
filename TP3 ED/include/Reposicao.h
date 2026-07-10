#ifndef REPOSICAO_H
#define REPOSICAO_H

#include "Vetor.h"
#include "ParIdQtd.h"

/*
 * Reposicao: registro de uma reposicao de estoque. Toda reposicao e valida e
 * adiciona unidades ao estoque dos produtos informados. O id e atribuido
 * automaticamente em ordem crescente a partir de 0. O vetor de itens guarda os
 * produtos repostos e suas quantidades, em ordem crescente de id de produto.
 */
struct Reposicao {
    int id;
    int timestamp;
    Vetor<ParIdQtd> itens;

    Reposicao();
    Reposicao(int id, int timestamp);
};

#endif // REPOSICAO_H
