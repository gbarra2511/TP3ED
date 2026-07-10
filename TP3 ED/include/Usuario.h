#ifndef USUARIO_H
#define USUARIO_H

#include <string>

/*
 * Usuario: entidade de quem realiza compras na plataforma. O id e atribuido
 * automaticamente pelo sistema, em ordem crescente a partir de 0. Os demais
 * atributos nao sao unicos e sao indexados para consulta multiatributo.
 */
struct Usuario {
    int id;
    std::string nome;
    int idade;
    std::string cidade;
    std::string estado;
    std::string nacionalidade;

    Usuario();
    Usuario(int id, const std::string& nome, int idade,
            const std::string& cidade, const std::string& estado,
            const std::string& nacionalidade);
};

#endif // USUARIO_H
