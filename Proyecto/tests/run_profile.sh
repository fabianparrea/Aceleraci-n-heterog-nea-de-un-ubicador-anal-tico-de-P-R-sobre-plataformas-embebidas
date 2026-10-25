#!/usr/bin/env bash
# Perfilado por etapa: corre cada benchmark de a uno (nunca dos a la vez, se
# contaminan los tiempos) y guarda la salida y el CSV por iteracion en
# results/<plataforma>/<config>_<hilos>t/.
#
# uso:  tests/run_profile.sh <cpu|gpu> [hilos] [benchmark.aux ...]
# ej.:  tests/run_profile.sh cpu 4
#       tests/run_profile.sh gpu 4 bench/ispd2005/adaptec1.aux
#       OUT_NAME=cpu_base_4t tests/run_profile.sh cpu 4   (otro nombre de carpeta)
#
# Antes de cada corrida espera a que la CPU baje de COOL_TEMP grados (como mucho
# COOL_WAIT segundos), para que todas arranquen con la placa en condiciones
# parecidas.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"

CONFIG=${1:-cpu}
THREADS=${2:-4}
if [ $# -gt 2 ]; then
    BENCHES=("${@:3}")
else
    BENCHES=(bench/ispd2005/adaptec1.aux bench/ispd2005/adaptec2.aux
             bench/ispd2005/bigblue1.aux bench/ispd2006/newblue1.aux)
fi

case "$CONFIG" in
    cpu) EXECUTABLE="$PROJECT_ROOT/build/placer" ;;
    gpu) EXECUTABLE="$PROJECT_ROOT/build/placer_gpu" ;;
    *)   echo "[!] config desconocida '$CONFIG' (usar cpu o gpu)"; exit 1 ;;
esac
if [ ! -x "$EXECUTABLE" ]; then
    echo "[!] falta $EXECUTABLE: compilar antes con 'make' (cpu) o 'make gpu' (gpu)"
    exit 1
fi
# en el modo de 5 W la Jetson apaga dos nucleos y baja el reloj: la medicion no
# seria la que dice el nombre de la carpeta. (nproc no sirve para contar los
# nucleos: devuelve OMP_NUM_THREADS si esta exportada)
ONLINE=$(getconf _NPROCESSORS_ONLN)
if [ "$ONLINE" -lt "$THREADS" ]; then
    echo "[!] se pidieron $THREADS hilos y hay $ONLINE nucleos en linea (revisar nvpmodel)"
    exit 1
fi
# en la Jetson: modo MAXN y relojes fijos (jetson_clocks deja el minimo igual al
# maximo). La salida de nvpmodel se guarda antes de buscar MAXN: con pipefail, un
# codigo de error de nvpmodel (sin root avisa que no puede leer el tope de la
# memoria) haria fallar el chequeo aunque el modo sea el correcto
if [ -f /etc/nv_tegra_release ] && [ "${ALLOW_LOW_POWER:-0}" != 1 ]; then
    MODE=$(nvpmodel -q 2> /dev/null || true)
    CPU0=/sys/devices/system/cpu/cpu0/cpufreq
    case "$MODE" in
        *MAXN*) ;;
        *)  echo "[!] la Jetson no esta en MAXN: sudo nvpmodel -m 0 && sudo jetson_clocks"
            echo "    (ALLOW_LOW_POWER=1 para medir igual)"
            exit 1 ;;
    esac
    if [ "$(cat "$CPU0/scaling_min_freq")" != "$(cat "$CPU0/scaling_max_freq")" ]; then
        echo "[!] los relojes no estan fijos: sudo jetson_clocks (ALLOW_LOW_POWER=1 para medir igual)"
        exit 1
    fi
fi

# en la Jetson los resultados quedan en results/jetson, en otra maquina con su nombre
if [ -f /etc/nv_tegra_release ]; then
    PLATFORM=${PLATFORM:-jetson}
else
    PLATFORM=${PLATFORM:-$(hostname)}
fi
OUT_DIR="$PROJECT_ROOT/results/$PLATFORM/${OUT_NAME:-${CONFIG}_${THREADS}t}"
mkdir -p "$OUT_DIR"
COOL_TEMP=${COOL_TEMP:-45}
COOL_WAIT=${COOL_WAIT:-300}

# temperatura de cada zona termica, en grados. PMIC-Die se omite: en la Jetson
# Nano devuelve un valor fijo que no es una medicion
temps() {
    local z t type
    for z in /sys/class/thermal/thermal_zone*; do
        t=$(cat "$z/temp" 2> /dev/null) || continue
        [[ $t =~ ^-?[0-9]+$ ]] || continue
        type=$(cat "$z/type" 2> /dev/null) || continue
        [ "$type" = PMIC-Die ] && continue
        printf '%s=%s ' "$type" "$(( t / 1000 ))"
    done
    return 0
}

# temperatura de la CPU (CPU-therm en la Jetson; si no existe, la zona mas caliente)
cpu_temp() {
    local z t type max=0
    for z in /sys/class/thermal/thermal_zone*; do
        t=$(cat "$z/temp" 2> /dev/null) || continue
        [[ $t =~ ^[0-9]+$ ]] || continue
        type=$(cat "$z/type" 2> /dev/null) || continue
        if [ "$type" = CPU-therm ]; then
            echo $(( t / 1000 ))
            return 0
        fi
        [ "$type" != PMIC-Die ] && [ $(( t / 1000 )) -gt "$max" ] && max=$(( t / 1000 ))
    done
    echo "$max"
}

wait_cool() {
    local waited=0
    while [ "$(cpu_temp)" -gt "$COOL_TEMP" ] && [ "$waited" -lt "$COOL_WAIT" ]; do
        sleep 5
        waited=$(( waited + 5 ))
    done
    if [ "$(cpu_temp)" -gt "$COOL_TEMP" ]; then
        echo "    (la CPU sigue a $(cpu_temp) C despues de $COOL_WAIT s; se corre igual)"
    fi
    return 0
}

# frecuencia actual de cada nucleo, en MHz
cpu_freqs() {
    local c
    for c in /sys/devices/system/cpu/cpu[0-9]*/cpufreq; do
        [ -r "$c/scaling_cur_freq" ] && printf '%s ' "$(( $(cat "$c/scaling_cur_freq") / 1000 ))"
    done
    return 0
}

# governor y rango de reloj de la CPU y la GPU: con jetson_clocks el minimo
# queda igual al maximo
clock_settings() {
    local c g
    for c in /sys/devices/system/cpu/cpu[0-9]*/cpufreq; do
        [ -r "$c/scaling_min_freq" ] || continue
        printf '%s: %s %s-%s MHz\n' "$(basename "$(dirname "$c")")" "$(cat "$c/scaling_governor")" \
            "$(( $(cat "$c/scaling_min_freq") / 1000 ))" "$(( $(cat "$c/scaling_max_freq") / 1000 ))"
    done
    for g in /sys/devices/gpu.0/devfreq/*; do
        [ -r "$g/min_freq" ] || continue
        printf 'gpu: %s %s-%s MHz\n' "$(cat "$g/governor")" \
            "$(( $(cat "$g/min_freq") / 1000000 ))" "$(( $(cat "$g/max_freq") / 1000000 ))"
    done
    return 0
}

# cada linea de runs.txt lleva el hash del binario: si una carpeta mezcla
# corridas de dos sesiones (un reintento parcial), se nota
SHA=$(sha256sum "$EXECUTABLE" | cut -c1-16)

{
    echo "fecha: $(date -Iseconds)"
    echo "binario: $EXECUTABLE"
    echo "sha256: $(sha256sum "$EXECUTABLE" | cut -d' ' -f1)"
    echo "hilos (OMP_NUM_THREADS): $THREADS"
    uname -a
    [ -f /etc/nv_tegra_release ] && head -1 /etc/nv_tegra_release
    gcc --version | head -1
    for nvcc in nvcc /usr/local/cuda/bin/nvcc; do
        if command -v "$nvcc" > /dev/null; then
            "$nvcc" --version | tail -1
            break
        fi
    done
    echo "nucleos en linea: $(cat /sys/devices/system/cpu/online 2> /dev/null || nproc)"
    clock_settings
    command -v nvpmodel > /dev/null && { nvpmodel -q 2> /dev/null || true; }
    if [ -f "$PROJECT_ROOT/VERSION" ]; then
        echo "version: $(cat "$PROJECT_ROOT/VERSION")"
    elif git -C "$PROJECT_ROOT" rev-parse HEAD > /dev/null 2>&1; then
        echo "version: $(git -C "$PROJECT_ROOT" describe --always --dirty)" \
             "($(git -C "$PROJECT_ROOT" status --porcelain | wc -l) archivos sin commitear)"
    fi
} > "$OUT_DIR/system.txt" 2>&1 || true

# si el script se corta (Ctrl-C, un error) tegrastats no puede quedar corriendo
TEGRA_PID=""
stop_tegrastats() {
    if [ -n "$TEGRA_PID" ]; then
        kill "$TEGRA_PID" 2> /dev/null || true
        wait "$TEGRA_PID" 2> /dev/null || true
        TEGRA_PID=""
    fi
}
trap stop_tegrastats EXIT
trap 'exit 130' INT TERM

echo "=================================================="
echo " Config:   $CONFIG ($THREADS hilos)"
echo " Salida:   $OUT_DIR"
echo "=================================================="

for REL in "${BENCHES[@]}"; do
    if [[ "$REL" = /* ]]; then AUX="$REL"; else AUX="$PROJECT_ROOT/$REL"; fi
    if [ ! -f "$AUX" ]; then
        echo "[!] no existe $AUX"
        exit 1
    fi
    BENCH_DIR="$(dirname "$AUX")"
    NAME="$(basename "$AUX" .aux)"

    if pgrep -x placer > /dev/null || pgrep -x placer_gpu > /dev/null; then
        echo "[!] ya hay otro placer corriendo y los tiempos se contaminarian; se aborta"
        exit 1
    fi

    wait_cool
    BEFORE="carga=$(cut -d' ' -f1 /proc/loadavg) cpu_mhz=$(cpu_freqs)temp: $(temps)"
    echo "[+] $NAME  ($BEFORE)"
    rm -f "$OUT_DIR/$NAME".*

    if command -v tegrastats > /dev/null; then
        tegrastats --interval 1000 --logfile "$OUT_DIR/$NAME.tegrastats.log" > /dev/null 2>&1 &
        TEGRA_PID=$!
    fi

    STATUS=0
    ( cd "$BENCH_DIR" && OMP_NUM_THREADS=$THREADS "$EXECUTABLE" "$NAME.aux" ) \
        > "$OUT_DIR/$NAME.log" 2>&1 || STATUS=$?
    stop_tegrastats

    if [ "$STATUS" -ne 0 ]; then
        echo "[!] $NAME fallo (codigo $STATUS), ver $OUT_DIR/$NAME.log"
        exit "$STATUS"
    fi

    # el CSV y las salidas de la corrida quedan juntos (.out.pl y .ppm los ignora git)
    mv "$BENCH_DIR/$NAME.prof.csv" "$OUT_DIR/$NAME.csv"
    mv "$BENCH_DIR/$NAME.out.pl" "$BENCH_DIR/$NAME.ppm" "$OUT_DIR/" 2> /dev/null || true
    AFTER="temp: $(temps)"
    echo "$(date -Iseconds) $NAME sha256=$SHA antes: $BEFORE | despues: $AFTER" >> "$OUT_DIR/runs.txt"
    grep -E "GPU:|HPWL legalizado|tiempos por etapa" "$OUT_DIR/$NAME.log" | sed 's/^/    /'
    echo "    $AFTER"
done

echo "[+] listo: $OUT_DIR"
