#include "MercadoDCC.h"
#include <iostream>
#include <cstdio>  // snprintf
#include <cstdlib> // atoi, atof
#include "Stats.h"
#ifdef TP_STATS
#include <chrono>
#endif

// Funcoes auxiliares de uso interno (sem ligacao externa).
namespace {
    int paraInt(const std::string& s) {
        return std::atoi(s.c_str());
    }

    double paraDouble(const std::string& s) {
        return std::atof(s.c_str());
    }

    // Converte o preco para centavos inteiros, evitando comparacao de double.
    int precoCentavos(double preco) {
        return (int)(preco * 100.0 + 0.5);
    }

    // Formata o preco sempre com duas casas decimais.
    std::string formatarPreco(double preco) {
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%.2f", preco);
        return std::string(buf);
    }

    // Devolve uma copia da posting list apontada, ou uma lista vazia se nula.
    Vetor<int> copiaOuVazio(const Vetor<int>* p) {
        if (p == nullptr) {
            Vetor<int> vazio;
            return vazio;
        }
        return *p;
    }
}

MercadoDCC::MercadoDCC() {}

// ===================== Manutencao das estruturas =====================

void MercadoDCC::acumularHistorico(Vetor<ParIdQtd>& lista, int id, int qtd) {
    size_t pos = 0;
    while (pos < lista.tamanho() && lista[pos].id < id) {
        pos++;
    }
    if (pos < lista.tamanho() && lista[pos].id == id) {
        lista[pos].qtd += qtd; // ja existe: soma a quantidade
    } else {
        lista.inserir(pos, ParIdQtd(id, qtd)); // novo: insere ordenado
    }
}

void MercadoDCC::inserirItemOrdenado(Vetor<ParIdQtd>& itens, int id, int qtd) {
    size_t pos = 0;
    while (pos < itens.tamanho() && itens[pos].id < id) {
        pos++;
    }
    itens.inserir(pos, ParIdQtd(id, qtd));
}

// ===================== Operacoes de conjunto =====================

bool MercadoDCC::ehNumero(const std::string& s) {
    if (s.empty()) return false;
    bool temDigito = false;
    int pontos = 0;
    for (size_t k = 0; k < s.size(); k++) {
        char c = s[k];
        if (c >= '0' && c <= '9') {
            temDigito = true;
        } else if (c == '.') {
            pontos++;
        } else {
            return false;
        }
    }
    return temDigito && pontos <= 1;
}

// Interseção (AND) de duas listas ordenadas: elementos comuns.
Vetor<int> MercadoDCC::intersecao(const Vetor<int>& a, const Vetor<int>& b) {
    Vetor<int> r;
    size_t i = 0, j = 0;
    while (i < a.tamanho() && j < b.tamanho()) {
        if (a[i] < b[j]) {
            i++;
        } else if (b[j] < a[i]) {
            j++;
        } else {
            r.adicionar(a[i]);
            i++;
            j++;
        }
    }
    return r;
}

// Uniao (OR) de duas listas ordenadas, sem duplicatas.
Vetor<int> MercadoDCC::uniao(const Vetor<int>& a, const Vetor<int>& b) {
    Vetor<int> r;
    size_t i = 0, j = 0;
    while (i < a.tamanho() && j < b.tamanho()) {
        if (a[i] < b[j]) {
            r.adicionar(a[i]);
            i++;
        } else if (b[j] < a[i]) {
            r.adicionar(b[j]);
            j++;
        } else {
            r.adicionar(a[i]);
            i++;
            j++;
        }
    }
    while (i < a.tamanho()) {
        r.adicionar(a[i]);
        i++;
    }
    while (j < b.tamanho()) {
        r.adicionar(b[j]);
        j++;
    }
    return r;
}

// Complemento (NOT) de uma lista ordenada em relacao ao universo [0, n).
Vetor<int> MercadoDCC::complemento(const Vetor<int>& a, int n) {
    Vetor<int> r;
    size_t i = 0;
    for (int v = 0; v < n; v++) {
        if (i < a.tamanho() && a[i] == v) {
            i++; // v esta em 'a', nao entra no complemento
        } else {
            r.adicionar(v);
        }
    }
    return r;
}

// ===================== Atributos auxiliares de consulta =====================

bool MercadoDCC::temOperadores(const Vetor<std::string>& t) const {
    for (size_t i = 1; i < t.tamanho(); i++) {
        if (t[i] == "AND" || t[i] == "OR" || t[i] == "NOT") {
            return true;
        }
    }
    return false;
}

bool MercadoDCC::ehAtributoFaixa(int tipo, const std::string& attr) const {
    if (tipo == 0) return attr == "idade";
    if (tipo == 1) return attr == "preco" || attr == "qtd";
    if (tipo == 2) return attr == "timestamp";
    if (tipo == 3) return attr == "timestamp";
    return false;
}

int MercadoDCC::universo(int tipo) const {
    if (tipo == 0) return (int) usuarios.tamanho();
    if (tipo == 1) return (int) produtos.tamanho();
    if (tipo == 2) return (int) compras.tamanho();
    return (int) reposicoes.tamanho();
}

// ===================== Posting lists por filtro =====================

Vetor<int> MercadoDCC::postingExato(int tipo, const std::string& attr,
                                    const std::string& valor) const {
    Vetor<int> vazio;

    if (tipo == 0) { // usuario
        if (attr == "id") {
            int v = paraInt(valor);
            if (v >= 0 && v < (int) usuarios.tamanho()) {
                Vetor<int> r;
                r.adicionar(v);
                return r;
            }
            return vazio;
        }
        if (attr == "nome") return copiaOuVazio(idxNomeUsuario.buscar(valor));
        if (attr == "idade") return copiaOuVazio(idxIdade.buscar(paraInt(valor)));
        if (attr == "cidade") return copiaOuVazio(idxCidade.buscar(valor));
        if (attr == "estado") return copiaOuVazio(idxEstado.buscar(valor));
        if (attr == "nacionalidade") return copiaOuVazio(idxNacionalidade.buscar(valor));
        return vazio;
    }

    if (tipo == 1) { // produto
        if (attr == "id") {
            int v = paraInt(valor);
            if (v >= 0 && v < (int) produtos.tamanho()) {
                Vetor<int> r;
                r.adicionar(v);
                return r;
            }
            return vazio;
        }
        if (attr == "nome") return copiaOuVazio(idxNomeProduto.buscar(valor));
        if (attr == "categoria") return copiaOuVazio(idxCategoria.buscar(valor));
        if (attr == "marca") return copiaOuVazio(idxMarca.buscar(valor));
        if (attr == "condicao") return copiaOuVazio(idxCondicao.buscar(valor));
        if (attr == "preco") return copiaOuVazio(idxPreco.buscar(precoCentavos(paraDouble(valor))));
        if (attr == "qtd") { // sem indice: varredura sobre o estoque atual
            Vetor<int> r;
            int q = paraInt(valor);
            for (size_t k = 0; k < produtos.tamanho(); k++) {
                if (produtos[k].qtd == q) r.adicionar((int) k);
            }
            return r;
        }
        return vazio;
    }

    if (tipo == 2) { // compra
        if (attr == "id") {
            int v = paraInt(valor);
            if (v >= 0 && v < (int) compras.tamanho()) {
                Vetor<int> r;
                r.adicionar(v);
                return r;
            }
            return vazio;
        }
        if (attr == "timestamp") return copiaOuVazio(idxTimestampCompra.buscar(paraInt(valor)));
        if (attr == "id_usuario") {
            int v = paraInt(valor);
            if (v >= 0 && v < (int) comprasPorUsuario.tamanho()) return comprasPorUsuario[v];
            return vazio;
        }
        if (attr == "id_produto") {
            int v = paraInt(valor);
            if (v >= 0 && v < (int) comprasPorProduto.tamanho()) return comprasPorProduto[v];
            return vazio;
        }
        return vazio;
    }

    // tipo == 3: reposicao
    if (attr == "id") {
        int v = paraInt(valor);
        if (v >= 0 && v < (int) reposicoes.tamanho()) {
            Vetor<int> r;
            r.adicionar(v);
            return r;
        }
        return vazio;
    }
    if (attr == "timestamp") return copiaOuVazio(idxTimestampReposicao.buscar(paraInt(valor)));
    if (attr == "id_produto") {
        int v = paraInt(valor);
        if (v >= 0 && v < (int) reposicoesPorProduto.tamanho()) return reposicoesPorProduto[v];
        return vazio;
    }
    return vazio;
}

Vetor<int> MercadoDCC::postingFaixa(int tipo, const std::string& attr,
                                    const std::string& vmin, const std::string& vmax) const {
    Vetor<int> resultado;

    if (tipo == 0 && attr == "idade") {
        Vetor<Vetor<int>*> col;
        idxIdade.coletarFaixa(paraInt(vmin), paraInt(vmax), col);
        for (size_t k = 0; k < col.tamanho(); k++) resultado = uniao(resultado, *col[k]);
        return resultado;
    }
    if (tipo == 1 && attr == "preco") {
        Vetor<Vetor<int>*> col;
        idxPreco.coletarFaixa(precoCentavos(paraDouble(vmin)),
                              precoCentavos(paraDouble(vmax)), col);
        for (size_t k = 0; k < col.tamanho(); k++) resultado = uniao(resultado, *col[k]);
        return resultado;
    }
    if (tipo == 1 && attr == "qtd") { // varredura sobre o estoque atual
        int lo = paraInt(vmin), hi = paraInt(vmax);
        for (size_t k = 0; k < produtos.tamanho(); k++) {
            if (produtos[k].qtd >= lo && produtos[k].qtd <= hi) resultado.adicionar((int) k);
        }
        return resultado; // ja ordenado por id
    }
    if (tipo == 2 && attr == "timestamp") {
        Vetor<Vetor<int>*> col;
        idxTimestampCompra.coletarFaixa(paraInt(vmin), paraInt(vmax), col);
        for (size_t k = 0; k < col.tamanho(); k++) resultado = uniao(resultado, *col[k]);
        return resultado;
    }
    if (tipo == 3 && attr == "timestamp") {
        Vetor<Vetor<int>*> col;
        idxTimestampReposicao.coletarFaixa(paraInt(vmin), paraInt(vmax), col);
        for (size_t k = 0; k < col.tamanho(); k++) resultado = uniao(resultado, *col[k]);
        return resultado;
    }
    return resultado; // atributo sem suporte a faixa
}

// ===================== Avaliador de consultas =====================

Vetor<int> MercadoDCC::parseAtomo(const Vetor<std::string>& t, size_t& i, int tipo) const {
    if (i >= t.tamanho()) {
        Vetor<int> vazio;
        return vazio;
    }
    const std::string attr = t[i];
    i++;
    if (i >= t.tamanho()) {
        Vetor<int> vazio;
        return vazio;
    }
    const std::string v1 = t[i];
    i++;
    // Faixa: atributo numerico de faixa seguido de dois valores numericos.
    if (ehAtributoFaixa(tipo, attr) && i < t.tamanho() && ehNumero(v1) && ehNumero(t[i])) {
        const std::string v2 = t[i];
        i++;
        return postingFaixa(tipo, attr, v1, v2);
    }
    return postingExato(tipo, attr, v1);
}

Vetor<int> MercadoDCC::parseNao(const Vetor<std::string>& t, size_t& i, int tipo) const {
    if (i < t.tamanho() && t[i] == "NOT") {
        i++; // consome NOT
        Vetor<int> a = parseAtomo(t, i, tipo);
        return complemento(a, universo(tipo));
    }
    return parseAtomo(t, i, tipo);
}

Vetor<int> MercadoDCC::parseE(const Vetor<std::string>& t, size_t& i, int tipo) const {
    Vetor<int> r = parseNao(t, i, tipo);
    while (i < t.tamanho() && t[i] == "AND") {
        i++; // consome AND
        Vetor<int> dir = parseNao(t, i, tipo);
        r = intersecao(r, dir);
    }
    return r;
}

Vetor<int> MercadoDCC::parseOu(const Vetor<std::string>& t, size_t& i, int tipo) const {
    Vetor<int> r = parseE(t, i, tipo);
    while (i < t.tamanho() && t[i] == "OR") {
        i++; // consome OR
        Vetor<int> dir = parseE(t, i, tipo);
        r = uniao(r, dir);
    }
    return r;
}

Vetor<int> MercadoDCC::avaliarConsulta(const Vetor<std::string>& t, int tipo) const {
    if (temOperadores(t)) {
        // Formato com operadores explicitos (ponto extra).
        size_t i = 1;
        return parseOu(t, i, tipo);
    }
    // Formato principal: todos os filtros combinados por AND (interseccao).
    size_t i = 1;
    if (i >= t.tamanho()) {
        Vetor<int> vazio;
        return vazio;
    }
    Vetor<int> resultado = parseAtomo(t, i, tipo);
    while (i < t.tamanho()) {
        Vetor<int> prox = parseAtomo(t, i, tipo);
        resultado = intersecao(resultado, prox);
    }
    return resultado;
}

// ===================== Comandos de escrita =====================

void MercadoDCC::cmdU(const Vetor<std::string>& t) {
    int id = (int) usuarios.tamanho();
    Usuario u(id, t[1], paraInt(t[2]), t[3], t[4], t[5]);
    usuarios.adicionar(u);

    idxNomeUsuario.inserir(t[1]).adicionar(id);
    idxIdade.inserir(u.idade).adicionar(id);
    idxCidade.inserir(t[3]).adicionar(id);
    idxEstado.inserir(t[4]).adicionar(id);
    idxNacionalidade.inserir(t[5]).adicionar(id);

    // Cresce as estruturas indexadas pelo id de usuario.
    comprasPorUsuario.adicionar(Vetor<int>());
    historicoUsuario.adicionar(Vetor<ParIdQtd>());

    std::cout << "U " << id << "\n";
}

void MercadoDCC::cmdP(const Vetor<std::string>& t) {
    int id = (int) produtos.tamanho();
    double preco = paraDouble(t[2]);
    Produto p(id, t[1], preco, paraInt(t[3]), t[4], t[5], t[6]);
    produtos.adicionar(p);

    idxNomeProduto.inserir(t[1]).adicionar(id);
    idxCategoria.inserir(t[4]).adicionar(id);
    idxMarca.inserir(t[5]).adicionar(id);
    idxCondicao.inserir(t[6]).adicionar(id);
    idxPreco.inserir(precoCentavos(preco)).adicionar(id); // ponto extra (faixa)

    // Cresce as estruturas indexadas pelo id de produto.
    comprasPorProduto.adicionar(Vetor<int>());
    reposicoesPorProduto.adicionar(Vetor<int>());
    historicoProduto.adicionar(Vetor<ParIdQtd>());

    std::cout << "P " << id << "\n";
}

void MercadoDCC::cmdR(const Vetor<std::string>& t) {
    int id = (int) reposicoes.tamanho();
    int ts = paraInt(t[1]);
    Reposicao r(id, ts);

    for (size_t i = 2; i + 1 < t.tamanho(); i += 2) {
        int p = paraInt(t[i]);
        int q = paraInt(t[i + 1]);
        inserirItemOrdenado(r.itens, p, q);
        produtos[p].qtd += q;             // reposicao adiciona ao estoque
        reposicoesPorProduto[p].adicionar(id);
    }

    reposicoes.adicionar(r);
    idxTimestampReposicao.inserir(ts).adicionar(id);

    std::cout << "R " << id << "\n";
}

void MercadoDCC::cmdC(const Vetor<std::string>& t) {
    int ts = paraInt(t[1]);
    int uid = paraInt(t[2]);

    // Valida: a compra so e valida se ha estoque para todos os produtos.
    bool valida = true;
    for (size_t i = 3; i + 1 < t.tamanho(); i += 2) {
        int p = paraInt(t[i]);
        int q = paraInt(t[i + 1]);
        if (produtos[p].qtd < q) {
            valida = false;
            break;
        }
    }
    if (!valida) {
        std::cout << "C INV\n";
        return;
    }

    // Efetiva a compra.
    int id = (int) compras.tamanho();
    Compra c(id, ts, uid);
    for (size_t i = 3; i + 1 < t.tamanho(); i += 2) {
        int p = paraInt(t[i]);
        int q = paraInt(t[i + 1]);
        produtos[p].qtd -= q;             // compra remove do estoque
        inserirItemOrdenado(c.itens, p, q);
        comprasPorProduto[p].adicionar(id);
        acumularHistorico(historicoUsuario[uid], p, q);
        acumularHistorico(historicoProduto[p], uid, q);
    }

    compras.adicionar(c);
    idxTimestampCompra.inserir(ts).adicionar(id);
    comprasPorUsuario[uid].adicionar(id);

    std::cout << "C " << id << "\n";
}

// ===================== Comandos de consulta =====================

void MercadoDCC::cmdLU(const Vetor<std::string>& t) {
    Vetor<int> ids = avaliarConsulta(t, 0);
    if (ids.tamanho() == 0) {
        std::cout << "LU VAZIO\n";
        return;
    }
    for (size_t k = 0; k < ids.tamanho(); k++) {
        const Usuario& u = usuarios[ids[k]];
        std::cout << "LU resultado_" << (k + 1) << " usuario " << u.id << " "
                  << u.nome << " " << u.idade << " " << u.cidade << " "
                  << u.estado << " " << u.nacionalidade << "\n";

        const Vetor<ParIdQtd>& h = historicoUsuario[ids[k]];
        if (h.tamanho() > 0) {
            for (size_t m = 0; m < h.tamanho(); m++) {
                std::cout << "produto_" << (m + 1) << " " << h[m].id << " " << h[m].qtd;
                if (m + 1 < h.tamanho()) std::cout << " ";
            }
            std::cout << "\n";
        }
    }
}

void MercadoDCC::cmdLP(const Vetor<std::string>& t) {
    Vetor<int> ids = avaliarConsulta(t, 1);
    if (ids.tamanho() == 0) {
        std::cout << "LP VAZIO\n";
        return;
    }
    for (size_t k = 0; k < ids.tamanho(); k++) {
        const Produto& p = produtos[ids[k]];
        std::cout << "LP resultado_" << (k + 1) << " produto " << p.id << " "
                  << p.nome << " " << formatarPreco(p.preco) << " " << p.qtd << " "
                  << p.categoria << " " << p.marca << " " << p.condicao << "\n";

        const Vetor<ParIdQtd>& h = historicoProduto[ids[k]];
        if (h.tamanho() > 0) {
            for (size_t m = 0; m < h.tamanho(); m++) {
                std::cout << "usuario_" << (m + 1) << " " << h[m].id << " " << h[m].qtd;
                if (m + 1 < h.tamanho()) std::cout << " ";
            }
            std::cout << "\n";
        }
    }
}

void MercadoDCC::cmdLC(const Vetor<std::string>& t) {
    Vetor<int> ids = avaliarConsulta(t, 2);
    if (ids.tamanho() == 0) {
        std::cout << "LC VAZIO\n";
        return;
    }
    for (size_t k = 0; k < ids.tamanho(); k++) {
        const Compra& c = compras[ids[k]];
        std::cout << "LC resultado_" << (k + 1) << " compra " << c.id
                  << " timestamp " << c.timestamp << " usuario " << c.idUsuario << "\n";
        for (size_t m = 0; m < c.itens.tamanho(); m++) {
            std::cout << "produto_" << (m + 1) << " " << c.itens[m].id << " " << c.itens[m].qtd;
            if (m + 1 < c.itens.tamanho()) std::cout << " ";
        }
        std::cout << "\n";
    }
}

void MercadoDCC::cmdLR(const Vetor<std::string>& t) {
    Vetor<int> ids = avaliarConsulta(t, 3);
    if (ids.tamanho() == 0) {
        std::cout << "LR VAZIO\n";
        return;
    }
    for (size_t k = 0; k < ids.tamanho(); k++) {
        const Reposicao& r = reposicoes[ids[k]];
        std::cout << "LR resultado_" << (k + 1) << " reposicao " << r.id
                  << " timestamp " << r.timestamp << "\n";
        for (size_t m = 0; m < r.itens.tamanho(); m++) {
            std::cout << "produto_" << (m + 1) << " " << r.itens[m].id << " " << r.itens[m].qtd;
            if (m + 1 < r.itens.tamanho()) std::cout << " ";
        }
        std::cout << "\n";
    }
}

// ===================== Despachante =====================

void MercadoDCC::processar(const Vetor<std::string>& t) {
    if (t.tamanho() == 0) return;
    const std::string& cmd = t[0];

#ifdef TP_STATS
    int idxStat = -1;
    if (cmd == "U") idxStat = 0;
    else if (cmd == "P") idxStat = 1;
    else if (cmd == "R") idxStat = 2;
    else if (cmd == "C") idxStat = 3;
    else if (cmd == "LU") idxStat = 4;
    else if (cmd == "LP") idxStat = 5;
    else if (cmd == "LC") idxStat = 6;
    else if (cmd == "LR") idxStat = 7;
    std::chrono::high_resolution_clock::time_point ini =
        std::chrono::high_resolution_clock::now();
#endif

    if (cmd == "U") cmdU(t);
    else if (cmd == "P") cmdP(t);
    else if (cmd == "R") cmdR(t);
    else if (cmd == "C") cmdC(t);
    else if (cmd == "LU") cmdLU(t);
    else if (cmd == "LP") cmdLP(t);
    else if (cmd == "LC") cmdLC(t);
    else if (cmd == "LR") cmdLR(t);
    // Comando desconhecido e ignorado (a entrada e valida quanto ao formato).

#ifdef TP_STATS
    std::chrono::high_resolution_clock::time_point fim =
        std::chrono::high_resolution_clock::now();
    if (idxStat >= 0) {
        g_stats.tempoTotal[idxStat] +=
            std::chrono::duration<double, std::micro>(fim - ini).count();
        g_stats.contagem[idxStat]++;
    }
#endif
}
