#ifndef PRODUTO_H
#define PRODUTO_H

#include <string>

/*
 * Produto: item vendavel da plataforma. O id e atribuido automaticamente em
 * ordem crescente a partir de 0. O campo qtd representa o estoque atual, que
 * cai em compras validas e sobe em reposicoes. O preco tem sempre duas casas
 * decimais. Os atributos qtd e preco nao sao usados como filtros na
 * especificacao principal (apenas como faixa nos pontos extras).
 */
struct Produto {
    int id;
    std::string nome;
    double preco;
    int qtd;
    std::string categoria;
    std::string marca;
    std::string condicao;

    Produto();
    Produto(int id, const std::string& nome, double preco, int qtd,
            const std::string& categoria, const std::string& marca,
            const std::string& condicao);
};

#endif // PRODUTO_H
