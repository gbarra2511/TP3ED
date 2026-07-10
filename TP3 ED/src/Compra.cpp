#include "Compra.h"

// O vetor de itens nasce vazio; os produtos sao adicionados ao processar o comando.
Compra::Compra()
    : id(0), timestamp(0), idUsuario(0) {}

Compra::Compra(int id, int timestamp, int idUsuario)
    : id(id), timestamp(timestamp), idUsuario(idUsuario) {}
