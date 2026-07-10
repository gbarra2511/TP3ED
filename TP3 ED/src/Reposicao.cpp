#include "Reposicao.h"

// O vetor de itens nasce vazio; os produtos sao adicionados ao processar o comando.
Reposicao::Reposicao()
    : id(0), timestamp(0) {}

Reposicao::Reposicao(int id, int timestamp)
    : id(id), timestamp(timestamp) {}
