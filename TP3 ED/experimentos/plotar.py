#!/usr/bin/env python3
# Le os CSVs de dados/ e gera os graficos em graficos/.
# Usa apenas o modulo csv (sem pandas) para facilitar a reexecucao.

import os
import csv
import math
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

AQUI = os.path.dirname(os.path.abspath(__file__))
DADOS = os.path.join(AQUI, "dados")
GRAF = os.path.join(AQUI, "graficos")
os.makedirs(GRAF, exist_ok=True)

COR = {"cadastros": "#2563eb", "compras": "#16a34a", "consultas": "#d97706"}


def ler(nome):
    with open(os.path.join(DADOS, nome)) as f:
        return list(csv.DictReader(f))


def salvar(fig, nome):
    fig.tight_layout()
    fig.savefig(os.path.join(GRAF, nome), dpi=150)
    plt.close(fig)
    print("  gravado:", nome)


# 1) Tempo medio por operacao, nos tres perfis (escala log no eixo y).
def g_perfis_tempo():
    dados = ler("perfis_tempo.csv")
    ops = ["U", "P", "R", "C", "LU", "LP", "LC", "LR"]
    perfis = ["cadastros", "compras", "consultas"]
    valores = {p: {o: 0.0 for o in ops} for p in perfis}
    for d in dados:
        valores[d["perfil"]][d["op"]] = float(d["media_us"])
    x = range(len(ops))
    largura = 0.26
    fig, ax = plt.subplots(figsize=(9, 5))
    for i, p in enumerate(perfis):
        ax.bar([xi + (i - 1) * largura for xi in x],
               [valores[p][o] for o in ops], largura, label=p, color=COR[p])
    ax.set_yscale("log")
    ax.set_xticks(list(x))
    ax.set_xticklabels(ops)
    ax.set_ylabel("tempo medio por operacao (us, escala log)")
    ax.set_xlabel("operacao")
    ax.set_title("Tempo medio por operacao nos tres perfis de carga")
    ax.legend()
    ax.grid(axis="y", alpha=0.3)
    salvar(fig, "01_perfis_tempo.png")


# 2) Contadores estruturais por perfil (escala log).
def g_perfis_contadores():
    dados = ler("perfis_contadores.csv")
    metricas = [("vetor_acessos", "Vetor: acessos"),
                ("hash_comp", "Hash: comparacoes"),
                ("avl_comp", "AVL: comparacoes")]
    perfis = [d["perfil"] for d in dados]
    x = range(len(metricas))
    largura = 0.26
    fig, ax = plt.subplots(figsize=(9, 5))
    for i, d in enumerate(dados):
        ax.bar([xi + (i - 1) * largura for xi in x],
               [int(d[m[0]]) for m in metricas], largura,
               label=d["perfil"], color=COR[d["perfil"]])
    ax.set_yscale("log")
    ax.set_xticks(list(x))
    ax.set_xticklabels([m[1] for m in metricas])
    ax.set_ylabel("contagem total (escala log)")
    ax.set_title("Custo estrutural por perfil de carga")
    ax.legend()
    ax.grid(axis="y", alpha=0.3)
    salvar(fig, "02_perfis_contadores.png")


# 3) Escalabilidade da insercao: custo por insercao vs n.
def g_escala_insercao():
    dados = ler("escala_insercao.csv")
    n = [int(d["n"]) for d in dados]
    avl = [float(d["avl_comp_por_ins"]) for d in dados]
    hsh = [float(d["hash_comp_por_ins"]) for d in dados]
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(n, avl, "o-", color="#2563eb", label="AVL (indice de idade)")
    ax.plot(n, hsh, "s-", color="#16a34a", label="Hash (indices textuais)")
    ax.set_xscale("log")
    ax.set_ylim(0, max(avl + hsh) * 1.4)
    ax.set_xlabel("numero de usuarios n (escala log)")
    ax.set_ylabel("comparacoes por insercao")
    ax.set_title("Custo de insercao por indice: constante porque o dominio\n"
                 "de valores (idades, cidades...) e limitado, nao cresce com n")
    ax.legend()
    ax.grid(alpha=0.3)
    salvar(fig, "03_escala_insercao.png")


# 4) Escalabilidade da consulta LU vs n.
def g_escala_consulta():
    dados = ler("escala_consulta.csv")
    n = [int(d["n"]) for d in dados]
    lu = [float(d["lu_media_us"]) for d in dados]
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.plot(n, lu, "o-", color="#d97706")
    ax.set_xlabel("numero de usuarios n")
    ax.set_ylabel("tempo medio da consulta LU (us)")
    ax.set_title("Custo da consulta LU cresce com n\n"
                 "(posting lists maiores -> mais trabalho de intersecao)")
    ax.grid(alpha=0.3)
    salvar(fig, "04_escala_consulta.png")


# 5) Custo da consulta por numero de filtros.
def g_filtros():
    dados = ler("filtros.csv")
    k = [d["filtros"] for d in dados]
    lu = [float(d["lu_media_us"]) for d in dados]
    fig, ax = plt.subplots(figsize=(7, 5))
    ax.bar(k, lu, color="#7c3aed", width=0.6)
    ax.set_xlabel("numero de filtros na consulta")
    ax.set_ylabel("tempo medio da consulta LU (us)")
    ax.set_title("Efeito do numero de filtros no custo da consulta")
    ax.grid(axis="y", alpha=0.3)
    salvar(fig, "05_filtros.png")


# 6) AVL vs BST: o grafico-chave da comparacao de estruturas.
def g_avl_bst():
    dados = ler("avl_bst.csv")
    def serie(ordem, estrutura, campo):
        pts = [(int(d["n"]), float(d[campo])) for d in dados
               if d["ordem"] == ordem and d["estrutura"] == estrutura]
        pts.sort()
        return [p[0] for p in pts], [p[1] for p in pts]

    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(13, 5))

    # Comparacoes (log-log).
    for est, cor in [("avl", "#2563eb"), ("bst", "#dc2626")]:
        nx, cy = serie("crescente", est, "comparacoes")
        ax1.plot(nx, cy, "o-", color=cor, label=est.upper() + " (crescente)")
        nx, cy = serie("aleatorio", est, "comparacoes")
        ax1.plot(nx, cy, "s--", color=cor, alpha=0.5, label=est.upper() + " (aleatorio)")
    ax1.set_xscale("log"); ax1.set_yscale("log")
    ax1.set_xlabel("numero de chaves n (log)")
    ax1.set_ylabel("comparacoes (log)")
    ax1.set_title("Comparacoes na insercao\nBST cresce O(n^2) no caso crescente; AVL O(n log n)")
    ax1.legend(fontsize=8); ax1.grid(alpha=0.3, which="both")

    # Tempo (log-log).
    for est, cor in [("avl", "#2563eb"), ("bst", "#dc2626")]:
        nx, ty = serie("crescente", est, "tempo_us")
        ax2.plot(nx, ty, "o-", color=cor, label=est.upper() + " (crescente)")
    ax2.set_xscale("log"); ax2.set_yscale("log")
    ax2.set_xlabel("numero de chaves n (log)")
    ax2.set_ylabel("tempo total (us, log)")
    ax2.set_title("Tempo de construcao (insercao crescente)\nAVL permanece viavel; BST degenera")
    ax2.legend(fontsize=8); ax2.grid(alpha=0.3, which="both")

    salvar(fig, "06_avl_vs_bst.png")


# 7) Ponto extra: custo da consulta por faixa vs largura.
def g_faixa():
    dados = ler("faixa.csv")
    w = [int(d["largura"]) for d in dados]
    lu = [float(d["lu_media_us"]) for d in dados]
    vis = [int(d["avl_visitas"]) for d in dados]
    fig, ax1 = plt.subplots(figsize=(8, 5))
    ax1.plot(w, lu, "o-", color="#0891b2", label="tempo medio (us)")
    ax1.set_xlabel("largura da faixa de idade")
    ax1.set_ylabel("tempo medio da consulta LU (us)", color="#0891b2")
    ax1.tick_params(axis="y", labelcolor="#0891b2")
    ax2 = ax1.twinx()
    ax2.plot(w, vis, "s--", color="#be185d", label="visitas na AVL")
    ax2.set_ylabel("nos visitados na AVL", color="#be185d")
    ax2.tick_params(axis="y", labelcolor="#be185d")
    ax1.set_title("Ponto extra: custo da consulta por faixa cresce com a largura")
    ax1.grid(alpha=0.3)
    salvar(fig, "07_extra_faixa.png")


# 8) Ponto extra: custo por forma de consulta booleana.
def g_booleano():
    dados = ler("booleano.csv")
    rotulos = {"and": "A AND B", "or": "A OR B",
               "not": "NOT A AND B", "mix": "NOT A AND B OR C"}
    formas = [rotulos[d["forma"]] for d in dados]
    lu = [float(d["lu_media_us"]) for d in dados]
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.bar(formas, lu, color="#059669", width=0.6)
    ax.set_ylabel("tempo medio da consulta LU (us)")
    ax.set_title("Ponto extra: custo por forma de consulta booleana")
    ax.grid(axis="y", alpha=0.3)
    plt.setp(ax.get_xticklabels(), rotation=15)
    salvar(fig, "08_extra_booleano.png")


if __name__ == "__main__":
    g_perfis_tempo()
    g_perfis_contadores()
    g_escala_insercao()
    g_escala_consulta()
    g_filtros()
    g_avl_bst()
    g_faixa()
    g_booleano()
    print("Graficos gerados em", GRAF)
