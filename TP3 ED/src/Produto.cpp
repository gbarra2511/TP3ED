#include "Produto.h"

// Construtor padrao: zera os campos numericos (as strings ja nascem vazias).
Produto::Produto()
    : id(0), preco(0.0), qtd(0) {}

Produto::Produto(int id, const std::string& nome, double preco, int qtd,
                 const std::string& categoria, const std::string& marca,
                 const std::string& condicao)
    : id(id), nome(nome), preco(preco), qtd(qtd),
      categoria(categoria), marca(marca), condicao(condicao) {}
