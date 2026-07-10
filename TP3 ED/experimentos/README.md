# Análise Experimental — TP3 MercadoDCC

Suíte de experimentos para a seção de Análise Experimental do relatório. Toda a
medição usa o **mesmo código-fonte** do TP, compilado com instrumentação via
`make stats` (macro `-DTP_STATS`). O binário de submissão (`make all`) não tem
nenhum custo de medição.

## Como reexecutar (no seu Mac)

```sh
cd ../TP && make stats        # binario instrumentado em bin/tp3.out
cd ../experimentos
python3 gerar_cenarios.py     # escreve cenarios/ (entradas deterministicas)
python3 coletar.py            # roda os cenarios -> dados/*.csv
g++ -std=c++11 -O2 -DTP_STATS -I../TP/include bench_avl_bst.cpp -o bench
./bench > dados/avl_bst.csv   # comparacao AVL vs BST
python3 plotar.py             # graficos/*.png
```

> Os **tempos** dependem da máquina — rode no seu Mac para os números do
> relatório. Os **contadores** (comparações, acessos, rotações) são
> independentes de hardware e reproduzíveis.

## Experimentos e gráficos

| Gráfico | Experimento | O que mostra | Seção do relatório |
|---|---|---|---|
| 01 | 3 perfis de carga | Tempo por operação (cadastros / compras / consultas) | Análise Experimental |
| 02 | 3 perfis de carga | Custo estrutural (acessos, comparações) por perfil | Análise Experimental |
| 03 | Escala da inserção | Custo de inserção por índice **constante em n** | Complexidade + Experimental |
| 04 | Escala da consulta | Custo de LU cresce com n (posting lists maiores) | Experimental |
| 05 | Nº de filtros | Efeito do número de filtros na consulta | Experimental |
| 06 | **AVL vs BST** | BST degenera para O(n²) com chaves crescentes; AVL O(n log n) | Método + Experimental |
| 07 | Ponto extra: faixa | Custo da consulta por faixa cresce com a largura | Experimental (extra) |
| 08 | Ponto extra: booleano | Custo por forma de consulta (AND/OR/NOT) | Experimental (extra) |

## Achados principais (para a discussão do relatório)

1. **AVL vs BST (gráfico 06) — o resultado mais forte.** Inserindo `timestamp`
   em ordem crescente (caso real, já que o tempo lógico cresce), uma BST simples
   degenera para O(n²): em 100 mil chaves faz ~5 bilhões de comparações e leva
   ~10 s, contra ~1,5 milhão de comparações e ~7 ms da AVL. Isso justifica
   diretamente a escolha da AVL para os índices numéricos.

2. **Custo de inserção constante (gráfico 03).** As comparações por inserção nos
   índices ficam estáveis (~5 na AVL de idade, ~4 na hash) **independentemente de
   n**. Motivo: a cardinalidade de valores distintos é limitada pelo domínio do
   atributo (idades, cidades...), não por n. O crescimento com n aparece nas
   posting lists (append O(1)) e no tamanho dos resultados das consultas.

3. **Consultas dominam o tempo (gráficos 01, 04).** Escritas custam ~1 us;
   consultas LU/LP custam dezenas a centenas de us, pois fazem interseção das
   posting lists e imprimem os históricos. O custo cresce com o tamanho da base.

4. **Pontos extras (gráficos 07, 08).** A consulta por faixa escala com a largura
   (mais nós visitados na AVL e resultados maiores); as formas booleanas têm
   custo coerente com o número de operações de conjunto (uniões/complementos).

## Observação sobre localidade de referência

As posting lists e os vetores de entidades são contíguos em memória e percorridos
sequencialmente nas interseções — o que favorece a localidade espacial. O contador
`Vetor acessos` mede esse percurso. Uma lista encadeada teria a mesma complexidade
assintótica, mas pior tempo prático por espalhar os nós na memória.
