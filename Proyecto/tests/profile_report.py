#!/usr/bin/env python3
"""Reporte del perfilado por etapa (salida de tests/run_profile.sh).

uso: tests/profile_report.py <dir_base> [<dir_otro> ...] > reporte.md

Cada directorio es una configuracion (por ejemplo results/jetson/cpu_4t) con un
<benchmark>.csv por circuito (una fila por iteracion, tiempos en ms) y su .log.
El primero es la linea base contra la que se calcula el speedup de los demas.
Solo usa la biblioteca estandar, para poder correrlo tambien en la Jetson.
"""

import csv
import os
import re
import statistics
import sys

# en el orden en que se muestran; las sub-etapas van con sangria
STAGES = [
    ("wirelength", "wirelength", 0),
    ("wl_kernel", "kernel WA (GPU)", 1),
    ("density", "densidad", 0),
    ("splat", "reparto de area", 1),
    ("poisson", "Poisson (DCT ida + vuelta)", 1),
    ("field", "campo + sumas", 1),
    ("density_grad", "gradiente", 1),
    ("h2d", "copias host->GPU (dentro de wirelength y densidad)", 1),
    ("d2h", "copias GPU->host (dentro de wirelength y densidad)", 1),
    ("lambda", "calibracion de lambda", 0),
    ("hpwl", "HPWL", 0),
    ("update", "combinar + paso + Nesterov", 0),
]
# lo que esta fuera del ciclo de Nesterov: una muestra por corrida, sale del .log.
# "preparacion" es lo de run_placement que no es initial_place ni el ciclo: la
# grilla, crear el backend (en la GPU: contexto, subir el netlist, planes de
# cuFFT) y las dos evaluaciones de arranque.
ONE_SHOT = ["parseo", "initial_place", "preparacion", "nesterov", "legalizacion", "escritura", "total"]


def read_csv(path):
    with open(path, newline="") as f:
        rows = list(csv.DictReader(f))
    return {col: [float(r[col]) for r in rows] for col in rows[0] if col != "iter"} if rows else {}


def read_log(path):
    info = {}
    if not os.path.exists(path):
        return info
    text = open(path).read()
    m = re.search(r"tiempos por etapa \(s\):(.*)", text)
    if m:
        for key, val in re.findall(r"(\w+)=([\d.]+)", m.group(1)):
            info[key] = float(val)
    m = re.search(r"HPWL legalizado: ([\d.e+]+)", text)
    if m:
        info["hpwl_legal"] = float(m.group(1))
    m = re.search(r"HPWL sin legalizar: ([\d.e+]+)\s+overflow: ([\d.]+)\s+tiempo: ([\d.]+) s", text)
    if m:
        info["hpwl_global"], info["overflow"] = float(m.group(1)), float(m.group(2))
        placement = float(m.group(3))
        if "initial_place" in info and "nesterov" in info:
            info["preparacion"] = max(placement - info["initial_place"] - info["nesterov"], 0.0)
        if all(k in info for k in ("parseo", "legalizacion", "escritura")):
            info["total"] = info["parseo"] + placement + info["legalizacion"] + info["escritura"]
    m = re.search(r"GPU: (.*), lista en ([\d.]+) s", text)
    if m:
        info["gpu"], info["gpu_init"] = m.group(1), float(m.group(2))
    return info


# tegrastats deja una linea por segundo, por ejemplo:
#   RAM 974/3964MB ... CPU [99%@1479,50%@1479,52%@1479,50%@1479] ... GR3D_FREQ 0% ...
#   CPU@45C ... GPU@44C ... POM_5V_IN 4083/3517 POM_5V_GPU 163/163 POM_5V_CPU 2486/1920
# (POM_* en mW: valor actual / promedio desde que arranco tegrastats)
def read_tegrastats(path):
    if not os.path.exists(path):
        return {}
    cpu, gpu, ram, t_cpu, t_gpu, mhz, p_in = [], [], [], [], [], [], []
    for line in open(path):
        m = re.search(r"CPU \[([^\]]*)\]", line)
        if m:
            cores = [c for c in m.group(1).split(",") if "%" in c]
            if cores:
                cpu.append(sum(float(c.split("%")[0]) for c in cores) / len(cores))
                mhz += [float(c.split("@")[1]) for c in cores if "@" in c]
        m = re.search(r"GR3D_FREQ (\d+)%", line)
        if m:
            gpu.append(float(m.group(1)))
        m = re.search(r"RAM (\d+)/\d+MB", line)
        if m:
            ram.append(float(m.group(1)))
        m = re.search(r" CPU@([\d.]+)C", line)
        if m:
            t_cpu.append(float(m.group(1)))
        m = re.search(r" GPU@([\d.]+)C", line)
        if m:
            t_gpu.append(float(m.group(1)))
        m = re.search(r"POM_5V_IN (\d+)/", line)
        if m:
            p_in.append(float(m.group(1)))
    if not cpu:
        return {}
    info = {"samples": len(cpu), "cpu_mean": sum(cpu) / len(cpu)}
    if gpu:
        info["gpu_mean"], info["gpu_max"] = sum(gpu) / len(gpu), max(gpu)
    if ram:
        info["ram_max"] = max(ram)
    if t_cpu:
        info["t_cpu_max"] = max(t_cpu)
    if t_gpu:
        info["t_gpu_max"] = max(t_gpu)
    if mhz:
        info["mhz_min"] = min(mhz)
    if p_in:
        # una muestra por segundo: la energia es la suma de las potencias
        info["power_mean"] = sum(p_in) / len(p_in) / 1000.0
        info["energy_kj"] = sum(p_in) / 1e6
    return info


# percentil con interpolacion lineal entre los dos vecinos; a mano porque
# statistics.quantiles no existe en el Python 3.6 de la Jetson
def percentile(sorted_values, p):
    k = (len(sorted_values) - 1) * p
    lo = int(k)
    hi = min(lo + 1, len(sorted_values) - 1)
    return sorted_values[lo] + (sorted_values[hi] - sorted_values[lo]) * (k - lo)


def stats(values):
    s = sorted(values)
    return {
        "n": len(values),
        "mean": sum(values) / len(values),
        "median": statistics.median(values),
        "std": statistics.stdev(values) if len(values) >= 2 else 0.0,
        "p5": percentile(s, 0.05),
        "p95": percentile(s, 0.95),
        "sum": sum(values),
    }


def load_config(directory):
    benches = {}
    for name in sorted(os.listdir(directory)):
        if not name.endswith(".csv"):
            continue
        bench = name[:-4]
        cols = read_csv(os.path.join(directory, name))
        if not cols:
            continue
        # las columnas que nunca se usaron (por ejemplo las de GPU en la version CPU) se omiten
        st = {c: stats(v) for c, v in cols.items() if any(x > 0 for x in v)}
        benches[bench] = {
            "stats": st,
            "log": read_log(os.path.join(directory, bench + ".log")),
            "tegra": read_tegrastats(os.path.join(directory, bench + ".tegrastats.log")),
        }
    return benches


def stage_table(name, data):
    st = data["stats"]
    total = st["total"]["sum"]
    out = [f"#### {name}\n",
           f"Iteraciones muestreadas: {st['total']['n']} (una muestra por etapa en cada una).\n",
           "| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |",
           "|---|---|---|---|---|---|---|---|"]
    for key, label, indent in STAGES:
        if key not in st:
            continue
        s = st[key]
        lab = ("&nbsp;&nbsp;&nbsp;" + label) if indent else label
        out.append(f"| {lab} | {s['n']} | {s['mean']:.2f} | {s['median']:.2f} | {s['std']:.2f} | "
                   f"{s['p5']:.2f} | {s['p95']:.2f} | {100 * s['sum'] / total:.1f} |")
    s = st["total"]
    out.append(f"| **iteracion completa** | {s['n']} | {s['mean']:.2f} | {s['median']:.2f} | "
               f"{s['std']:.2f} | {s['p5']:.2f} | {s['p95']:.2f} | 100.0 |")
    return "\n".join(out) + "\n"


def coverage_line(name, data):
    st = data["stats"]
    total = st["total"]["sum"]
    pct = lambda keys: 100 * sum(st[k]["sum"] for k in keys if k in st) / total
    return (f"| {name} | {pct(['wirelength', 'density']):.1f} | "
            f"{pct(['wirelength', 'density', 'lambda']):.1f} | {pct(['lambda']):.1f} |")


def main():
    dirs = [d.rstrip("/") for d in sys.argv[1:]]
    if not dirs:
        print(__doc__, file=sys.stderr)
        sys.exit(1)
    configs = [(os.path.basename(d), load_config(d)) for d in dirs]
    base_name, base = configs[0]

    print("# Perfilado por etapa\n")
    print("Tiempos de pared por iteracion del ciclo de Nesterov (una fila del CSV por iteracion). "
          "En la version GPU las partes de cada etapa se miden con eventos de CUDA y las etapas "
          "completas con el reloj del host, que incluye el costo de lanzar y esperar.\n")

    for cfg, benches in configs:
        print(f"## {cfg}\n")
        for bench, data in benches.items():
            print(stage_table(bench, data))

    print("## Cobertura de las funciones descargadas (linea base: %s)\n" % base_name)
    print("Porcentaje del tiempo del ciclo que ocupan las funciones, para el criterio de "
          "cubrir al menos el 50 %.\n")
    print("| Circuito | wirelength + densidad (%) | + calibracion de lambda (%) | solo lambda (%) |")
    print("|---|---|---|---|")
    for bench, data in base.items():
        print(coverage_line(bench, data))
    print()

    for cfg, benches in configs[1:]:
        print(f"## Speedup de {cfg} contra {base_name}\n")
        print("Cociente de las medias por iteracion (la media es la que define el tiempo total).\n")
        print("| Circuito | Etapa | %s (ms) | %s (ms) | Speedup |" % (base_name, cfg))
        print("|---|---|---|---|---|")
        for bench, data in benches.items():
            if bench not in base:
                continue
            b, o = base[bench]["stats"], data["stats"]
            for key in ("wirelength", "density", "lambda", "hpwl", "update", "total"):
                if key in b and key in o:
                    label = "iteracion completa" if key == "total" else key
                    print(f"| {bench} | {label} | {b[key]['mean']:.2f} | {o[key]['mean']:.2f} | "
                          f"{b[key]['mean'] / o[key]['mean']:.2f}x |")
        print()

    for cfg, benches in configs[1:]:
        print(f"## Tiempo total de la corrida: {cfg} contra {base_name}\n")
        print("| Circuito | %s (s) | %s (s) | Speedup |" % (base_name, cfg))
        print("|---|---|---|---|")
        for bench, data in benches.items():
            b, o = base.get(bench, {}).get("log", {}), data["log"]
            if "total" in b and "total" in o:
                print(f"| {bench} | {b['total']:.1f} | {o['total']:.1f} | {b['total'] / o['total']:.2f}x |")
        print()

    print("## Etapas fuera del ciclo y calidad\n")
    print("Una muestra por corrida (del .log). Tiempos en segundos.\n")
    print("| Config | Circuito | " + " | ".join(ONE_SHOT) + " | GPU lista | HPWL global | overflow | HPWL legalizado |")
    print("|---|---|" + "---|" * (len(ONE_SHOT) + 4))
    for cfg, benches in configs:
        for bench, data in benches.items():
            log = data["log"]
            cells = [f"{log[k]:.2f}" if k in log else "-" for k in ONE_SHOT]
            gpu = f"{log['gpu_init']:.2f}" if "gpu_init" in log else "-"
            hg = f"{log['hpwl_global']:.4e}" if "hpwl_global" in log else "-"
            ov = f"{log['overflow']:.3f}" if "overflow" in log else "-"
            hl = f"{log['hpwl_legal']:.4e}" if "hpwl_legal" in log else "-"
            print(f"| {cfg} | {bench} | " + " | ".join(cells) + f" | {gpu} | {hg} | {ov} | {hl} |")
    print()
    resources_table(configs)


def resources_table(configs):
    rows = [(cfg, bench, data["tegra"]) for cfg, benches in configs
            for bench, data in benches.items() if data["tegra"]]
    if not rows:
        return
    fmt = lambda t, k, f: (f % t[k]) if k in t else "-"
    print("## Recursos durante cada corrida (tegrastats)\n")
    print("Una muestra por segundo durante toda la corrida (parseo a escritura). CPU es el "
          "uso promedio de los cuatro nucleos; la potencia es la de entrada de la placa "
          "(POM_5V_IN) y la energia, su suma en el tiempo.\n")
    print("| Config | Circuito | Muestras | CPU media (%) | GPU media (%) | GPU max (%) | "
          "RAM de la placa max (MB) | T CPU max (C) | T GPU max (C) | Reloj CPU min (MHz) | "
          "Potencia media (W) | Energia (kJ) |")
    print("|---|---|---|---|---|---|---|---|---|---|---|---|")
    for cfg, bench, t in rows:
        print(f"| {cfg} | {bench} | {t['samples']} | {fmt(t, 'cpu_mean', '%.0f')} | "
              f"{fmt(t, 'gpu_mean', '%.0f')} | {fmt(t, 'gpu_max', '%.0f')} | "
              f"{fmt(t, 'ram_max', '%.0f')} | {fmt(t, 't_cpu_max', '%.1f')} | "
              f"{fmt(t, 't_gpu_max', '%.1f')} | {fmt(t, 'mhz_min', '%.0f')} | "
              f"{fmt(t, 'power_mean', '%.2f')} | {fmt(t, 'energy_kj', '%.2f')} |")
    print()


if __name__ == "__main__":
    main()
