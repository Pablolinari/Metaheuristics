import pandas as pd
import seaborn as sns
import matplotlib.pyplot as plt
import os

def main():
    fnames: list[str] = [f for f in os.listdir(".") if f.endswith(".csv")]
    
    for fname in fnames:
        df = pd.read_csv(fname)
        df['alg'] = df['alg'].str.upper()

        # --- Gráfico 1: BOXPLOT (usando catplot) ---
        p1 = sns.catplot(data=df, x="alg", y="fitness", aspect=2, kind="box")
        p1.set(xlabel="Algoritmo", ylabel="Fitness")
        p1.savefig(fname.replace(".csv", ".png"))
        plt.close() # Importante: cerrar para liberar memoria

        # --- Gráfico 2: LINEPLOT ---
        # Creamos la figura explícitamente para evitar errores
        plt.figure(figsize=(14, 8))
        
        df['intento'] = df.groupby('alg').cumcount() + 1
        
        # Guardamos el resultado en 'ax' (es un objeto Axes)
        ax = sns.lineplot(
            data=df, 
            x="intento", 
            y="fitness", 
            hue="alg", 
            marker=None
        )
        
        ax.set(xlabel="Evaluación", ylabel="Fitness")
        ax.set_title(f"Evolución: {fname}")

        # SOLUCIÓN AL ERROR: Usamos plt.savefig() o ax.get_figure().savefig()
        foutput_linea = fname.replace(".csv", "_linea.png")
        plt.savefig(foutput_linea, bbox_inches='tight')
        
        plt.close() # Limpiamos el lienzo para el siguiente archivo CSV
        print(f"Procesado con éxito: {fname}")

if __name__ == '__main__':
    main()
