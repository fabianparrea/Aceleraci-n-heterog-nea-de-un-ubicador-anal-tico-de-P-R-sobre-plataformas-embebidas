#!/usr/bin/env python3

import glob
import os
import sys
import pandas as pd


def load_and_clean_data(csv_files):
    all_dfs = []
    for filepath in csv_files:
        try:
            df = pd.read_csv(filepath)
            # Limpiar espacios en blanco en nombres de columnas e hilos
            df.columns = df.columns.str.strip()
            df["event"] = df["event"].str.strip()
            df["benchmark"] = df["benchmark"].str.strip()
            # Convertir 'value' a numérico
            df["value"] = pd.to_numeric(df["value"], errors="coerce")
            all_dfs.append(df)
        except Exception as e:
            print(f"[!] Error leyendo {filepath}: {e}", file=sys.stderr)

    if not all_dfs:
        print("[!] No se encontraron datos válidos en los CSV proporcionados.")
        sys.exit(1)

    return pd.concat(all_dfs, ignore_index=True)


def process_benchmarks(df):
    # Pivotar la tabla para tener eventos como columnas por (benchmark, run)
    pivoted = df.pivot_table(
        index=["benchmark", "run"], columns="event", values="value"
    ).reset_index()

    # Calcular métricas derivadas si existen las columnas necesarias
    if "instructions" in pivoted and "cycles" in pivoted:
        pivoted["IPC"] = pivoted["instructions"] / pivoted["cycles"]

    if "task-clock" in pivoted:
        # Convertir task-clock de milisegundos a segundos si aplica
        pivoted["Time (s)"] = pivoted["task-clock"] / 1000.0

    if "cache-misses" in pivoted and "cache-references" in pivoted:
        pivoted["Cache Miss %"] = (
            pivoted["cache-misses"] / pivoted["cache-references"]
        ) * 100

    if "branch-misses" in pivoted and "branch-instructions" in pivoted:
        pivoted["Branch Miss %"] = (
            pivoted["branch-misses"] / pivoted["branch-instructions"]
        ) * 100

    # Agrupar por benchmark y obtener promedio y desviación estándar
    summary_mean = pivoted.groupby("benchmark").mean(numeric_only=True)
    summary_std = pivoted.groupby("benchmark").std(numeric_only=True)

    # Construir dataframe final para el reporte
    report_rows = []

    for bench in summary_mean.index:
        row = {"Benchmark": bench}

        # Tiempo (s)
        if "Time (s)" in summary_mean.columns:
            mean_t = summary_mean.loc[bench, "Time (s)"]
            std_t = summary_std.loc[bench, "Time (s)"]
            row["Time (s)"] = f"{mean_t:.3f} ± {std_t:.3f}"
        elif "task-clock" in summary_mean.columns:
            mean_t = summary_mean.loc[bench, "task-clock"] / 1000.0
            std_t = summary_std.loc[bench, "task-clock"] / 1000.0
            row["Time (s)"] = f"{mean_t:.3f} ± {std_t:.3f}"

        # Cycles & Instructions (en Millones 'M' o Billones 'B')
        if "cycles" in summary_mean.columns:
            cyc = summary_mean.loc[bench, "cycles"] / 1e6
            row["Cycles (M)"] = f"{cyc:.2f}"

        if "instructions" in summary_mean.columns:
            inst = summary_mean.loc[bench, "instructions"] / 1e6
            row["Inst (M)"] = f"{inst:.2f}"

        # IPC
        if "IPC" in summary_mean.columns:
            row["IPC"] = f"{summary_mean.loc[bench, 'IPC']:.3f}"

        # Cache Misses
        if "Cache Miss %" in summary_mean.columns:
            row["Cache Miss %"] = (
                f"{summary_mean.loc[bench, 'Cache Miss %']:.2f}%"
            )
        elif "cache-misses" in summary_mean.columns:
            cm = summary_mean.loc[bench, "cache-misses"] / 1e6
            row["Cache Misses (M)"] = f"{cm:.2f}"

        # Branch Misses
        if "Branch Miss %" in summary_mean.columns:
            row["Branch Miss %"] = (
                f"{summary_mean.loc[bench, 'Branch Miss %']:.2f}%"
            )

        report_rows.append(row)

    return pd.DataFrame(report_rows)


def main():
    # Detectar archivos CSV pasados por argumento o buscar todos los .csv en tests/
    if len(sys.argv) > 1:
        csv_files = sys.argv[1:]
    else:
        script_dir = os.path.dirname(os.path.abspath(__file__))
        csv_files = glob.glob(os.path.join(script_dir, "*.csv"))

    if not csv_files:
        print(
            "Uso: python3 generate_report.py [archivo1.csv archivo2.csv ...]"
        )
        print("O coloca archivos .csv dentro de la carpeta actual.")
        sys.exit(1)

    df_raw = load_and_clean_data(csv_files)
    df_report = process_benchmarks(df_raw)

    print("\n### Resumen de Desempeño y Muestreo de Hardware (`perf`)\n")
    print(df_report.to_markdown(index=False))
    print("\n")


if __name__ == "__main__":
    main()