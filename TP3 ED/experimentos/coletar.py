#!/usr/bin/env python3
# Roda cada cenario no binario instrumentado (make stats) e extrai as metricas
# do stderr, gravando os resultados em CSVs na pasta dados/.

import os
import re
import subprocess

AQUI = os.path.dirname(os.path.abspath(__file__))
CENARIOS = os.path.join(AQUI, "cenarios")
DADOS = os.path.join(AQUI, "dados")
BIN = os.path.join(AQUI, "..", "bin", "tp3.out")


def rodar(cenario):
    """Executa o binario com o cenario e devolve as metricas do stderr."""
    caminho = os.path.join(CENARIOS, cenario)
    with open(caminho, "r") as entrada:
        res = subprocess.run([BIN], stdin=entrada,
                             stdout=subprocess.DEVNULL,
                             stderr=subprocess.PIPE, text=True)
    return parse(res.stderr)


def parse(texto):
    """Converte o stderr instrumentado em um dicionario de metricas."""
    m = {"cmd": {}, "vetor": {}, "hash": {}, "avl": {}}
    for linha in texto.splitlines():
        p = linha.split()
        if len(p) >= 5 and p[1] == "media_us":
            m["cmd"][p[0]] = {"media_us": float(p[2]), "chamadas": int(p[4])}
        elif p and p[0] == "Vetor":
            m["vetor"] = {"acessos": int(p[2]), "realocacoes": int(p[4]),
                          "copias_elem": int(p[6])}
        elif p and p[0] == "Hash":
            m["hash"] = {"funcao": int(p[2]), "comparacoes": int(p[4]),
                         "rehash": int(p[6])}
        elif p and p[0] == "AVL":
            m["avl"] = {"comparacoes": int(p[2]), "rotacoes": int(p[4]),
                        "visitas_faixa": int(p[6])}
    return m


def escrever_csv(nome, cabecalho, linhas):
    os.makedirs(DADOS, exist_ok=True)
    with open(os.path.join(DADOS, nome), "w") as f:
        f.write(cabecalho + "\n")
        for l in linhas:
            f.write(",".join(str(x) for x in l) + "\n")


def main():
    OPS = ["U", "P", "R", "C", "LU", "LP", "LC", "LR"]

    # 1) Perfis: tempo medio por operacao + contadores estruturais.
    perfis = [("cadastros", "perfil_cadastros.txt"),
              ("compras", "perfil_compras.txt"),
              ("consultas", "perfil_consultas.txt")]
    linhas_tempo = []
    linhas_cont = []
    for nome, arq in perfis:
        print("  perfil:", nome)
        m = rodar(arq)
        for op in OPS:
            if op in m["cmd"]:
                linhas_tempo.append([nome, op, "%.4f" % m["cmd"][op]["media_us"]])
        linhas_cont.append([nome, m["vetor"]["acessos"], m["hash"]["comparacoes"],
                            m["avl"]["comparacoes"], m["hash"]["rehash"],
                            m["avl"]["rotacoes"]])
    escrever_csv("perfis_tempo.csv", "perfil,op,media_us", linhas_tempo)
    escrever_csv("perfis_contadores.csv",
                 "perfil,vetor_acessos,hash_comp,avl_comp,hash_rehash,avl_rot",
                 linhas_cont)

    # 2a) Escalabilidade da insercao (so cadastros).
    NS = [1000, 2000, 5000, 10000, 20000, 50000]
    linhas = []
    for n in NS:
        print("  escala_ins:", n)
        m = rodar("escala_ins_%d.txt" % n)
        u = m["cmd"]["U"]
        avl_por_ins = m["avl"]["comparacoes"] / n
        hash_por_ins = m["hash"]["comparacoes"] / n
        linhas.append([n, "%.4f" % u["media_us"], "%.4f" % avl_por_ins,
                       "%.4f" % hash_por_ins])
    escrever_csv("escala_insercao.csv",
                 "n,u_media_us,avl_comp_por_ins,hash_comp_por_ins", linhas)

    # 2b) Escalabilidade da consulta LU (tempo medio, separado por comando).
    NSQ = [1000, 2000, 5000, 10000, 20000]
    linhas = []
    for n in NSQ:
        print("  escala_qry:", n)
        m = rodar("escala_qry_%d.txt" % n)
        linhas.append([n, "%.4f" % m["cmd"]["LU"]["media_us"]])
    escrever_csv("escala_consulta.csv", "n,lu_media_us", linhas)

    # 3) Custo da consulta por numero de filtros.
    linhas = []
    for k in [1, 2, 3, 4]:
        print("  filtros:", k)
        m = rodar("filtros_%d.txt" % k)
        linhas.append([k, "%.4f" % m["cmd"]["LU"]["media_us"]])
    escrever_csv("filtros.csv", "filtros,lu_media_us", linhas)

    # 4) Ponto extra: faixa numerica por largura.
    linhas = []
    for w in [2, 5, 10, 20, 40]:
        print("  faixa:", w)
        m = rodar("faixa_%d.txt" % w)
        linhas.append([w, "%.4f" % m["cmd"]["LU"]["media_us"],
                       m["avl"]["visitas_faixa"]])
    escrever_csv("faixa.csv", "largura,lu_media_us,avl_visitas", linhas)

    # 5) Ponto extra: formas booleanas.
    linhas = []
    for forma in ["and", "or", "not", "mix"]:
        print("  bool:", forma)
        m = rodar("bool_%s.txt" % forma)
        linhas.append([forma, "%.4f" % m["cmd"]["LU"]["media_us"]])
    escrever_csv("booleano.csv", "forma,lu_media_us", linhas)

    print("CSVs gravados em", DADOS)


if __name__ == "__main__":
    main()
