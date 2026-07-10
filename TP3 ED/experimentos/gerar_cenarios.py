#!/usr/bin/env python3
# Gera os arquivos de entrada para a analise experimental do TP3.
# Todos os cenarios produzem entrada VALIDA quanto ao formato: comandos C/R
# referenciam apenas usuarios/produtos ja criados; consultas usam atributos
# permitidos. A geracao e deterministica (seed fixa) para reprodutibilidade.

import os
import random

random.seed(42)

NOMES = ["Ana", "Bruno", "Carla", "Diego", "Elena", "Felipe", "Gabi", "Hugo",
         "Ines", "Joao", "Kelly", "Lucas", "Maria", "Nina", "Otto", "Paula",
         "Rui", "Sofia", "Tiago", "Vera"]
CIDADES = ["BeloHorizonte", "SaoPaulo", "RioDeJaneiro", "Curitiba",
           "PortoAlegre", "Salvador", "Recife", "Fortaleza"]
ESTADOS = ["MG", "SP", "RJ", "PR", "RS", "BA", "PE", "CE"]
NACS = ["Brasileira", "Argentina", "Chilena", "Uruguaia", "Portuguesa"]
NOMES_PROD = ["Mouse", "Teclado", "Monitor", "Notebook", "Livro", "Caneta",
              "Cadeira", "Mesa", "Fone", "Camera", "Cabo", "Webcam"]
CATEGORIAS = ["Informatica", "Livros", "Moveis", "Eletronicos", "Acessorios"]
MARCAS = ["Logitech", "Dell", "Razer", "Pearson", "Samsung", "HP", "Genius"]
CONDICOES = ["Novo", "Usado", "Recondicionado"]


class Gerador:
    """Mantem o estado (ids ja criados) para gerar comandos validos."""

    def __init__(self):
        self.nu = 0   # usuarios criados
        self.np = 0   # produtos criados
        self.ts = 0   # timestamp logico crescente

    def U(self):
        self.nu += 1
        return "U %s %d %s %s %s" % (
            random.choice(NOMES), random.randint(18, 70),
            random.choice(CIDADES), random.choice(ESTADOS), random.choice(NACS))

    def P(self):
        self.np += 1
        return "P %s %.2f %d %s %s %s" % (
            random.choice(NOMES_PROD), random.uniform(5, 500),
            random.randint(1, 50), random.choice(CATEGORIAS),
            random.choice(MARCAS), random.choice(CONDICOES))

    def _pares_produtos(self, qmax):
        k = random.randint(1, min(3, self.np))
        prods = random.sample(range(self.np), k)
        return " ".join("%d %d" % (p, random.randint(1, qmax)) for p in sorted(prods))

    def R(self):
        self.ts += 1
        return "R %d %s" % (self.ts, self._pares_produtos(10))

    def C(self):
        self.ts += 1
        u = random.randint(0, self.nu - 1)
        return "C %d %d %s" % (self.ts, u, self._pares_produtos(3))

    def LU(self, nf=1):
        atrs = random.sample(["cidade", "estado", "nacionalidade", "idade", "nome"],
                             min(nf, 5))
        partes = []
        for a in atrs:
            if a == "cidade": partes.append("cidade " + random.choice(CIDADES))
            elif a == "estado": partes.append("estado " + random.choice(ESTADOS))
            elif a == "nacionalidade": partes.append("nacionalidade " + random.choice(NACS))
            elif a == "idade": partes.append("idade %d" % random.randint(18, 70))
            elif a == "nome": partes.append("nome " + random.choice(NOMES))
        return "LU " + " ".join(partes)

    def LP(self, nf=1):
        atrs = random.sample(["categoria", "marca", "condicao", "nome"], min(nf, 4))
        partes = []
        for a in atrs:
            if a == "categoria": partes.append("categoria " + random.choice(CATEGORIAS))
            elif a == "marca": partes.append("marca " + random.choice(MARCAS))
            elif a == "condicao": partes.append("condicao " + random.choice(CONDICOES))
            elif a == "nome": partes.append("nome " + random.choice(NOMES_PROD))
        return "LP " + " ".join(partes)

    def LC(self):
        op = random.random()
        if op < 0.34 and self.nu > 0:
            return "LC id_usuario %d" % random.randint(0, self.nu - 1)
        if op < 0.67 and self.np > 0:
            return "LC id_produto %d" % random.randint(0, self.np - 1)
        return "LC timestamp %d" % random.randint(1, max(1, self.ts))

    def LR(self):
        if random.random() < 0.5 and self.np > 0:
            return "LR id_produto %d" % random.randint(0, self.np - 1)
        return "LR timestamp %d" % random.randint(1, max(1, self.ts))


def escrever(caminho, linhas):
    os.makedirs(os.path.dirname(caminho), exist_ok=True)
    with open(caminho, "w") as f:
        f.write("\n".join(linhas) + "\n")


def gerar_perfil(nome_arquivo, total, pesos, base_u=200, base_p=200):
    """Gera um perfil de carga com a mistura de operacoes dada por 'pesos'."""
    g = Gerador()
    linhas = []
    # Aquecimento: garante entidades para compras, reposicoes e consultas.
    for _ in range(base_u):
        linhas.append(g.U())
    for _ in range(base_p):
        linhas.append(g.P())
    ops = list(pesos.keys())
    w = list(pesos.values())
    while len(linhas) < total:
        op = random.choices(ops, weights=w, k=1)[0]
        if op == "U": linhas.append(g.U())
        elif op == "P": linhas.append(g.P())
        elif op == "R": linhas.append(g.R())
        elif op == "C": linhas.append(g.C())
        elif op == "LU": linhas.append(g.LU(random.randint(1, 3)))
        elif op == "LP": linhas.append(g.LP(random.randint(1, 3)))
        elif op == "LC": linhas.append(g.LC())
        elif op == "LR": linhas.append(g.LR())
    escrever(nome_arquivo, linhas)


def gerar_escala_insercao(nome_arquivo, n):
    """Apenas n cadastros de usuario: isola o custo de insercao nos indices."""
    g = Gerador()
    linhas = [g.U() for _ in range(n)]
    escrever(nome_arquivo, linhas)


def gerar_escala_consulta(nome_arquivo, n, q=1500):
    """n usuarios (aquecimento) seguidos de q consultas LU de 2 filtros."""
    g = Gerador()
    linhas = [g.U() for _ in range(n)]
    for _ in range(q):
        linhas.append(g.LU(2))
    escrever(nome_arquivo, linhas)


def gerar_filtros(nome_arquivo, k, base=8000, q=2000):
    """base usuarios + q consultas LU com exatamente k filtros."""
    g = Gerador()
    linhas = [g.U() for _ in range(base)]
    for _ in range(q):
        linhas.append(g.LU(k))
    escrever(nome_arquivo, linhas)


def gerar_faixa(nome_arquivo, largura, base=8000, q=1500):
    """Consultas por faixa de idade com largura fixa (ponto extra)."""
    g = Gerador()
    linhas = [g.U() for _ in range(base)]
    for _ in range(q):
        lo = random.randint(18, 70 - largura)
        linhas.append("LU idade %d %d" % (lo, lo + largura))
    escrever(nome_arquivo, linhas)


def gerar_booleano(nome_arquivo, forma, base=8000, q=1500):
    """Consultas booleanas de uma forma especifica (ponto extra)."""
    g = Gerador()
    linhas = [g.U() for _ in range(base)]
    for _ in range(q):
        c1 = "cidade " + random.choice(CIDADES)
        c2 = "estado " + random.choice(ESTADOS)
        c3 = "nacionalidade " + random.choice(NACS)
        if forma == "and":
            linhas.append("LU %s AND %s" % (c1, c2))
        elif forma == "or":
            linhas.append("LU %s OR %s" % (c1, c2))
        elif forma == "not":
            linhas.append("LU NOT %s AND %s" % (c3, c1))
        else:  # mix
            linhas.append("LU NOT %s AND %s OR %s" % (c3, c1, c2))
    escrever(nome_arquivo, linhas)


if __name__ == "__main__":
    D = os.path.join(os.path.dirname(__file__), "cenarios")

    # 1) Tres perfis de carga (~20 mil operacoes cada).
    gerar_perfil(os.path.join(D, "perfil_cadastros.txt"), 20000,
                 {"U": 35, "P": 35, "R": 5, "C": 10, "LU": 4, "LP": 4, "LC": 2, "LR": 1})
    gerar_perfil(os.path.join(D, "perfil_compras.txt"), 20000,
                 {"U": 8, "P": 8, "R": 30, "C": 35, "LU": 3, "LP": 3, "LC": 3, "LR": 2})
    gerar_perfil(os.path.join(D, "perfil_consultas.txt"), 20000,
                 {"U": 5, "P": 5, "R": 3, "C": 7, "LU": 20, "LP": 20, "LC": 15, "LR": 10})

    # 2) Escalabilidade (insercao e consulta) em funcao de n.
    NS = [1000, 2000, 5000, 10000, 20000, 50000]
    for n in NS:
        gerar_escala_insercao(os.path.join(D, "escala_ins_%d.txt" % n), n)
    NSQ = [1000, 2000, 5000, 10000, 20000]
    for n in NSQ:
        gerar_escala_consulta(os.path.join(D, "escala_qry_%d.txt" % n), n)

    # 3) Custo da consulta em funcao do numero de filtros.
    for k in [1, 2, 3, 4]:
        gerar_filtros(os.path.join(D, "filtros_%d.txt" % k), k)

    # 4) Ponto extra: faixas numericas de larguras variadas.
    for w in [2, 5, 10, 20, 40]:
        gerar_faixa(os.path.join(D, "faixa_%d.txt" % w), w)

    # 5) Ponto extra: formas de consulta booleana.
    for forma in ["and", "or", "not", "mix"]:
        gerar_booleano(os.path.join(D, "bool_%s.txt" % forma), forma)

    print("Cenarios gerados em", D)
