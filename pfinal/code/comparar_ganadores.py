#!/usr/bin/env python3
"""
Genera tablas que indican qué algoritmo gana en cada función del benchmark CEC2017.

Lee los ficheros results_cec2017_<dim>.xlsx de cada carpeta que empieza por
"results" (un algoritmo por carpeta). En cada xlsx se toma la fila del milestone
final (mayor presupuesto de evaluaciones), que contiene el error medio por
función. El algoritmo con menor error gana esa función.

Salida:
  - Una tabla por dimensión con el error de cada algoritmo y el ganador.
  - Un resumen con el número de funciones ganadas por cada algoritmo.
Las tablas se imprimen por pantalla y se guardan en ganadores_<dim>.csv,
junto con un resumen_victorias.csv.
"""

import glob
import os
import re

import pandas as pd

RESULT_GLOB = "results*/"
FUNC_COLS = [f"F{i:02d}" for i in range(1, 31)]


def nombre_algoritmo(carpeta):
    """results_abc-de/ -> abc-de"""
    base = os.path.basename(os.path.normpath(carpeta))
    return re.sub(r"^results_", "", base)


def cargar_finales():
    """Devuelve {dim: {algoritmo: Series con error final por función}}."""
    datos = {}
    for carpeta in sorted(glob.glob(RESULT_GLOB)):
        algo = nombre_algoritmo(carpeta)
        for xlsx in sorted(glob.glob(os.path.join(carpeta, "results_cec2017_*.xlsx"))):
            df = pd.read_excel(xlsx)
            if "dimension" not in df.columns:
                continue
            dim = int(df["dimension"].iloc[0])
            # fila del milestone final (mayor presupuesto)
            fila = df.loc[df["milestone"].idxmax()]
            errores = fila[FUNC_COLS].astype(float)
            datos.setdefault(dim, {})[algo] = errores
    return datos


def main():
    datos = cargar_finales()
    if not datos:
        print("No se encontraron ficheros xlsx en las carpetas results*.")
        return

    resumen = {}  # algoritmo -> {dim: nº victorias}

    for dim in sorted(datos):
        tabla = pd.DataFrame(datos[dim])  # filas: F01..F30, cols: algoritmos
        tabla.index.name = "Funcion"
        # el ganador es el de menor error en cada función
        ganador = tabla.idxmin(axis=1)
        tabla_out = tabla.copy()
        tabla_out["Ganador"] = ganador

        print(f"\n{'='*70}\nDimensión {dim}: error final por función (menor = mejor)\n{'='*70}")
        with pd.option_context("display.float_format", lambda v: f"{v:.4g}",
                               "display.max_columns", None,
                               "display.width", 200):
            print(tabla_out.to_string())

        tabla_out.to_csv(f"ganadores_{dim}.csv")

        conteo = ganador.value_counts()
        for algo in tabla.columns:
            resumen.setdefault(algo, {})[dim] = int(conteo.get(algo, 0))

    # resumen de victorias por algoritmo y dimensión
    res_df = pd.DataFrame(resumen).T.fillna(0).astype(int)
    res_df = res_df.reindex(sorted(res_df.columns), axis=1)
    res_df["TOTAL"] = res_df.sum(axis=1)
    res_df = res_df.sort_values("TOTAL", ascending=False)
    res_df.index.name = "Algoritmo"

    print(f"\n{'='*70}\nResumen: nº de funciones ganadas por algoritmo\n{'='*70}")
    print(res_df.to_string())
    res_df.to_csv("resumen_victorias.csv")
    print("\nFicheros generados: ganadores_<dim>.csv y resumen_victorias.csv")


if __name__ == "__main__":
    main()
