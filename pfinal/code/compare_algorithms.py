#!/usr/bin/env python3
"""Compara los resultados (fitness/error final) de cada algoritmo a partir de los
xlsx que hay en las carpetas que empiezan por "results".

Cada carpeta -> un algoritmo.
Cada fichero results_cec2017_<dim>.xlsx -> una dimension.
Las columnas F01..F30 son el error al optimo (menor = mejor) y se usa la fila
del ultimo milestone (presupuesto completo de evaluaciones).

Genera, por cada dimension:
  - Un grafico de barras agrupado comparando el error por funcion (escala log).
  - Una "tabla" de quien gana cada funcion + recuento de victorias por algoritmo.

Uso:
    python3 compare_algorithms.py [--dir .] [--out graficas]
"""
import argparse
import glob
import os

import numpy as np
import pandas as pd
import matplotlib.pyplot as plt


def find_algorithms(base):
    """Devuelve {nombre_algoritmo: ruta_carpeta} para carpetas que empiezan por results."""
    algos = {}
    for path in sorted(glob.glob(os.path.join(base, "results*"))):
        if os.path.isdir(path):
            name = os.path.basename(path).replace("results_", "").replace("results", "")
            algos[name or os.path.basename(path)] = path
    return algos


def load_final_row(folder, dim):
    """Lee el xlsx de una dimension y devuelve la fila del ultimo milestone (Serie F01..F30)."""
    pattern = os.path.join(folder, f"*{dim}.xlsx")
    matches = glob.glob(pattern)
    if not matches:
        return None
    df = pd.read_excel(matches[0])
    df = df.sort_values("milestone")
    final = df.iloc[-1]
    funcs = [c for c in df.columns if c.startswith("F")]
    return final[funcs]


def discover_dimensions(algos):
    dims = set()
    for folder in algos.values():
        for f in glob.glob(os.path.join(folder, "*.xlsx")):
            # nombre tipo results_cec2017_30.xlsx
            stem = os.path.splitext(os.path.basename(f))[0]
            tail = stem.split("_")[-1]
            if tail.isdigit():
                dims.add(int(tail))
    return sorted(dims)


def plot_dimension(dim, data, out_dir):
    """data: {algoritmo: Serie F01..F30}. Genera barras agrupadas + resumen de victorias."""
    algos = list(data.keys())
    funcs = list(next(iter(data.values())).index)
    n_func = len(funcs)
    n_algo = len(algos)

    matrix = np.array([[float(data[a][f]) for f in funcs] for a in algos])  # [algo, func]

    # ---- Grafico de barras agrupado (escala log) ----
    fig, ax = plt.subplots(figsize=(max(12, n_func * 0.5), 6))
    width = 0.8 / n_algo
    x = np.arange(n_func)
    # evita ceros/negativos en log
    plot_vals = np.where(matrix <= 0, 1e-12, matrix)
    for i, a in enumerate(algos):
        ax.bar(x + i * width, plot_vals[i], width, label=a)
    ax.set_yscale("log")
    ax.set_xticks(x + width * (n_algo - 1) / 2)
    ax.set_xticklabels(funcs, rotation=90, fontsize=8)
    ax.set_ylabel("Error al optimo (log, menor = mejor)")
    ax.set_title(f"Comparativa de fitness por funcion - dimension {dim}")
    ax.legend()
    fig.tight_layout()
    bar_path = os.path.join(out_dir, f"comparativa_dim{dim}_barras.png")
    fig.savefig(bar_path, dpi=120)
    plt.close(fig)

    # ---- Quien gana cada funcion + recuento ----
    winners = matrix.argmin(axis=0)  # indice del algoritmo ganador por funcion
    wins = {a: 0 for a in algos}
    for w in winners:
        wins[algos[w]] += 1

    # Grafico resumen de victorias
    fig, ax = plt.subplots(figsize=(8, 5))
    ax.bar(wins.keys(), wins.values(), color="tab:green")
    ax.set_ylabel("Nº de funciones ganadas")
    ax.set_title(f"Victorias por algoritmo - dimension {dim} ({n_func} funciones)")
    for i, (a, v) in enumerate(wins.items()):
        ax.text(i, v, str(v), ha="center", va="bottom")
    fig.tight_layout()
    win_path = os.path.join(out_dir, f"victorias_dim{dim}.png")
    fig.savefig(win_path, dpi=120)
    plt.close(fig)

    # ---- Resumen por consola ----
    print(f"\n=== Dimension {dim} ===")
    for f, w in zip(funcs, winners):
        vals = " | ".join(f"{a}={matrix[j, funcs.index(f)]:.3e}" for j, a in enumerate(algos))
        print(f"  {f}: gana {algos[w]:<12}  ({vals})")
    print("  --- Total de victorias ---")
    for a, v in sorted(wins.items(), key=lambda kv: -kv[1]):
        print(f"    {a}: {v}")
    print(f"  Graficas: {bar_path}  |  {win_path}")


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--dir", default=".", help="directorio base con las carpetas results*")
    ap.add_argument("--out", default="graficas", help="carpeta de salida para las imagenes")
    args = ap.parse_args()

    algos = find_algorithms(args.dir)
    if not algos:
        raise SystemExit("No se encontraron carpetas que empiecen por 'results'")
    print("Algoritmos detectados:", ", ".join(algos))

    os.makedirs(args.out, exist_ok=True)
    dims = discover_dimensions(algos)
    print("Dimensiones detectadas:", dims)

    for dim in dims:
        data = {}
        for name, folder in algos.items():
            row = load_final_row(folder, dim)
            if row is not None:
                data[name] = row
        if len(data) < 1:
            continue
        plot_dimension(dim, data, args.out)


if __name__ == "__main__":
    main()
