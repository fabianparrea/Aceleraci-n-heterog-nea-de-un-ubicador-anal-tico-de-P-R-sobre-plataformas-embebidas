#!/usr/bin/env bash

set -euo pipefail

# Determinar directorios clave
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

EXECUTABLE="$PROJECT_ROOT/build/placer"
OUTPUT_CSV="$SCRIPT_DIR/perf_results.csv"

# Parámetros de entrada
RUNS=${1:-5}
REL_BENCH=${2:-"bench/toy/toy.aux"}

# Resolver la ruta absoluta del benchmark
if [[ "$REL_BENCH" = /* ]]; then
    BENCHMARK_AUX="$REL_BENCH"
else
    BENCHMARK_AUX="$PROJECT_ROOT/$REL_BENCH"
fi

if [ ! -f "$BENCHMARK_AUX" ]; then
    echo "[!] ERROR: No se encontró el archivo benchmark en '$BENCHMARK_AUX'"
    exit 1
fi

BENCH_DIR="$(dirname "$BENCHMARK_AUX")"
BENCH_FILE="$(basename "$BENCHMARK_AUX")"
BENCH_NAME="$(basename "$BENCHMARK_AUX" .aux)"

# Si no existe el binario, compilar desde la raíz
if [ ! -f "$EXECUTABLE" ]; then
    echo "[+] Compilando el proyecto..."
    (cd "$PROJECT_ROOT" && make)
fi

# Lista de eventos estándar de perf
EVENTS="task-clock,cycles,instructions,cache-references,cache-misses,branch-instructions,branch-misses"

# Crear cabecera CSV si el archivo no existe
if [ ! -f "$OUTPUT_CSV" ]; then
    echo "timestamp,benchmark,run,event,value,unit" > "$OUTPUT_CSV"
fi

TIMESTAMP=$(date +"%Y-%m-%d_%H-%M-%S")

echo "=================================================="
echo " Running: $BENCHMARK_AUX"
echo " Runs:    $RUNS"
echo " CSV:     $OUTPUT_CSV"
echo "=================================================="

for run in $(seq 1 "$RUNS"); do
    echo -n "  -> Ejecución $run/$RUNS... "
    
    TMP_PERF=$(mktemp)
    TMP_STDOUT=$(mktemp)

    # Ejecutar perf stat desde el directorio del benchmark
    (
        cd "$BENCH_DIR"
        perf stat -x, \
                  -e "$EVENTS" \
                  -o "$TMP_PERF" \
                  "$EXECUTABLE" "$BENCH_FILE" > "$TMP_STDOUT" 2>&1
    )
    EXIT_CODE=$?

    if [ $EXIT_CODE -ne 0 ]; then
        echo "FALLÓ (Exit code $EXIT_CODE)"
        cat "$TMP_STDOUT"
        rm -f "$TMP_PERF" "$TMP_STDOUT"
        exit $EXIT_CODE
    fi

    # Extraer y guardar eventos válidos devueltos por perf stat
    awk -F',' -v ts="$TIMESTAMP" -v bench="$BENCH_NAME" -v r="$run" '
        NF >= 3 {
            val = $1; unit = $2; evt = $3
            
            gsub(/^ +| +$/, "", val)
            gsub(/^ +| +$/, "", unit)
            gsub(/^ +| +$/, "", evt)
            
            # Solo procesar líneas donde se obtuvo un evento válido
            if (evt != "" && val != "<not counted>" && val != "<not supported>") {
                print ts "," bench "," r "," evt "," val "," unit
            }
        }
    ' "$TMP_PERF" >> "$OUTPUT_CSV"

    rm -f "$TMP_PERF" "$TMP_STDOUT"
    echo "Completada."
done

echo ""
echo "[+] Proceso finalizado. Datos agregados a '$OUTPUT_CSV'."