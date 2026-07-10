#include "Usuario.h"

// Construtor padrao: zera os campos numericos (as strings ja nascem vazias).
Usuario::Usuario()
    : id(0), idade(0) {}

Usuario::Usuario(int id, const std::string& nome, int idade,
                 const std::string& cidade, const std::string& estado,
                 const std::string& nacionalidade)
    : id(id), nome(nome), idade(idade), cidade(cidade),
      estado(estado), nacionalidade(nacionalidade) {}
