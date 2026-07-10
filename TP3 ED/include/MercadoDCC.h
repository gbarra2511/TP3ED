#ifndef MERCADODCC_H
#define MERCADODCC_H

#include <cstddef> 
#include <string>
#include "Vetor.h"
#include "Hash.h"
#include "AVL.h"
#include "ParIdQtd.h"
#include "Usuario.h"
#include "Produto.h"
#include "Compra.h"
#include "Reposicao.h"

/*
 * MercadoDCC.h
 *
 * Classe que mantem todo o estado do sistema: as entidades, os indices
 * invertidos e os historicos agregados, alem de implementar as oito operacoes
 * (U, P, R, C, LU, LP, LC, LR). Os TADs sao definidos em seus proprios arquivos.
 *
 * O parametro 'tipo' usado nos metodos de consulta identifica a entidade:
 *   0 = usuario (LU), 1 = produto (LP), 2 = compra (LC), 3 = reposicao (LR).
 *
 * Convencoes de modelagem:
 *  - Entidades guardadas em vetores indexados pelo proprio id (acesso O(1)).
 *  - Atributos textuais indexados por tabela hash; numericos por AVL (o que
 *    tambem habilita a consulta por faixa dos pontos extras).
 *  - Chaves que ja sao ids contiguos usam vetores indexados diretos.
 *  - O valor de cada indice e uma posting list (Vetor<int> de ids), construida
 *    por append e por isso naturalmente ordenada.
 */
class MercadoDCC {
private:
    // -------- Entidades, indexadas pelo proprio id --------
    Vetor<Usuario> usuarios;
    Vetor<Produto> produtos;
    Vetor<Compra> compras;
    Vetor<Reposicao> reposicoes;

    // -------- Indices invertidos de usuario --------
    Hash<Vetor<int> > idxNomeUsuario;
    AVL<int, Vetor<int> > idxIdade;        // exato (principal) e faixa (extra)
    Hash<Vetor<int> > idxCidade;
    Hash<Vetor<int> > idxEstado;
    Hash<Vetor<int> > idxNacionalidade;

    // -------- Indices invertidos de produto --------
    Hash<Vetor<int> > idxNomeProduto;
    Hash<Vetor<int> > idxCategoria;
    Hash<Vetor<int> > idxMarca;
    Hash<Vetor<int> > idxCondicao;
    AVL<int, Vetor<int> > idxPreco;        // ponto extra (faixa); chave em centavos

    // -------- Indices de compra --------
    AVL<int, Vetor<int> > idxTimestampCompra;
    Vetor<Vetor<int> > comprasPorUsuario;  // [idUsuario] -> ids de compras
    Vetor<Vetor<int> > comprasPorProduto;  // [idProduto] -> ids de compras

    // -------- Indices de reposicao --------
    AVL<int, Vetor<int> > idxTimestampReposicao;
    Vetor<Vetor<int> > reposicoesPorProduto; // [idProduto] -> ids de reposicoes

    // -------- Historicos agregados (segunda linha de LU/LP) --------
    Vetor<Vetor<ParIdQtd> > historicoUsuario; // [idUsuario] -> (produto, qtd total)
    Vetor<Vetor<ParIdQtd> > historicoProduto; // [idProduto] -> (usuario, qtd total)

    // -------- Manutencao das estruturas --------
    // Acumula 'qtd' no par de 'id' da lista, mantendo-a ordenada por id.
    void acumularHistorico(Vetor<ParIdQtd>& lista, int id, int qtd);
    // Insere um item (id, qtd) na lista mantendo a ordem crescente de id.
    void inserirItemOrdenado(Vetor<ParIdQtd>& itens, int id, int qtd);

    // -------- Avaliacao de consultas --------
    // Avalia os filtros de uma consulta e devolve os ids resultantes (ordenados).
    Vetor<int> avaliarConsulta(const Vetor<std::string>& t, int tipo) const;
    // Posting list (copia ordenada) de um filtro exato e de um filtro de faixa.
    Vetor<int> postingExato(int tipo, const std::string& attr,
                            const std::string& valor) const;
    Vetor<int> postingFaixa(int tipo, const std::string& attr,
                            const std::string& vmin, const std::string& vmax) const;

    // Descida recursiva para o formato com operadores (precedencia NOT > AND > OR).
    Vetor<int> parseOu(const Vetor<std::string>& t, size_t& i, int tipo) const;
    Vetor<int> parseE(const Vetor<std::string>& t, size_t& i, int tipo) const;
    Vetor<int> parseNao(const Vetor<std::string>& t, size_t& i, int tipo) const;
    Vetor<int> parseAtomo(const Vetor<std::string>& t, size_t& i, int tipo) const;

    bool temOperadores(const Vetor<std::string>& t) const;
    bool ehAtributoFaixa(int tipo, const std::string& attr) const;
    int universo(int tipo) const;

    // -------- Operacoes de conjunto sobre posting lists ordenadas --------
    static bool ehNumero(const std::string& s);
    static Vetor<int> intersecao(const Vetor<int>& a, const Vetor<int>& b);
    static Vetor<int> uniao(const Vetor<int>& a, const Vetor<int>& b);
    static Vetor<int> complemento(const Vetor<int>& a, int n);

    // -------- Tratadores de cada comando --------
    void cmdU(const Vetor<std::string>& t);
    void cmdP(const Vetor<std::string>& t);
    void cmdR(const Vetor<std::string>& t);
    void cmdC(const Vetor<std::string>& t);
    void cmdLU(const Vetor<std::string>& t);
    void cmdLP(const Vetor<std::string>& t);
    void cmdLC(const Vetor<std::string>& t);
    void cmdLR(const Vetor<std::string>& t);

public:
    MercadoDCC();

    // Recebe os tokens de uma linha (ja separados) e despacha pelo primeiro token.
    void processar(const Vetor<std::string>& tokens);
};

#endif // MERCADODCC_H
