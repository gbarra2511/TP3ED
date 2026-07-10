#ifndef PARIDQTD_H
#define PARIDQTD_H

/*
 * ParIdQtd: par (id, quantidade), tipo de valor leve reaproveitado nos itens
 * de uma compra/reposicao (id = produto) e nos historicos agregados
 * (id = produto, ou id = usuario). Por ser trivial, e mantido header-only
 * com construtores inline.
 */
struct ParIdQtd {
    int id;
    int qtd;

    ParIdQtd() : id(0), qtd(0) {}
    ParIdQtd(int id, int qtd) : id(id), qtd(qtd) {}
};

#endif // PARIDQTD_H
