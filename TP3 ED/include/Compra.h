#ifndef COMPRA_H
#define COMPRA_H

#include "Vetor.h"
#include "ParIdQtd.h"

/*
 * Compra registro de uma compra valida feita por um usuario. O id e atribuido
 * automaticamente em ordem crescente, considerando apenas compras validas. O
 * vetor de itens guarda os produtos comprados e suas quantidades, mantido em
 * ordem crescente de id de produto para a saida das consultas
 */
struct Compra {
    int id;
    int timestamp;
    int idUsuario;
    Vetor<ParIdQtd> itens;

    Compra();
    Compra(int id, int timestamp, int idUsuario);
};

#endif 
