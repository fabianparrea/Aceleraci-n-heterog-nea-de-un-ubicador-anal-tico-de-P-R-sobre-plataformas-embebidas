# Perfilado por etapa

Tiempos de pared por iteracion del ciclo de Nesterov (una fila del CSV por iteracion). En la version GPU las partes de cada etapa se miden con eventos de CUDA y las etapas completas con el reloj del host, que incluye el costo de lanzar y esperar. Desde que la GPU trabaja en paralelo con la CPU (HPWL y la mediana de wirelength se calculan mientras la GPU sigue), las filas wirelength y densidad son solo lo que la CPU espera cada resultado, y las partes medidas en la GPU ya no suman al total de la iteracion.

## a2_e0_base_gpu_4t

#### adaptec1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 20.29 | 20.27 | 0.20 | 20.21 | 20.39 | 35.7 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 18.13 | 18.12 | 0.02 | 18.09 | 18.17 | 31.9 |
| densidad | 1000 | 4.94 | 4.91 | 0.20 | 4.86 | 5.03 | 8.7 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 0.91 | 0.90 | 0.01 | 0.90 | 0.92 | 1.6 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.11 | 0.01 | 1.09 | 1.13 | 1.9 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.4 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 0.87 | 0.87 | 0.01 | 0.86 | 0.88 | 1.5 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 1.74 | 1.71 | 0.06 | 1.69 | 1.84 | 3.1 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 1.93 | 1.89 | 0.27 | 1.85 | 1.98 | 3.4 |
| calibracion de lambda | 1000 | 16.64 | 16.75 | 1.54 | 14.11 | 19.05 | 29.3 |
| HPWL | 1000 | 6.05 | 4.84 | 1.52 | 4.81 | 7.86 | 10.6 |
| combinar + paso + Nesterov | 1000 | 8.90 | 8.73 | 0.25 | 8.66 | 9.24 | 15.7 |
| **iteracion completa** | 1000 | 56.83 | 56.64 | 2.20 | 53.36 | 60.49 | 100.0 |

#### adaptec2

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 26.86 | 26.85 | 0.12 | 26.79 | 26.96 | 37.0 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 24.40 | 24.39 | 0.03 | 24.36 | 24.45 | 33.6 |
| densidad | 1000 | 5.96 | 5.93 | 0.20 | 5.87 | 6.02 | 8.2 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 1.42 | 1.41 | 0.01 | 1.41 | 1.43 | 2.0 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.10 | 0.01 | 1.09 | 1.13 | 1.5 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.3 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.03 | 1.03 | 0.00 | 1.02 | 1.03 | 1.4 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 2.08 | 2.07 | 0.04 | 2.03 | 2.13 | 2.9 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 2.25 | 2.21 | 0.22 | 2.16 | 2.31 | 3.1 |
| calibracion de lambda | 1000 | 21.56 | 21.57 | 2.46 | 17.63 | 25.76 | 29.7 |
| HPWL | 1000 | 7.32 | 6.07 | 1.68 | 6.02 | 10.11 | 10.1 |
| combinar + paso + Nesterov | 1000 | 10.93 | 11.14 | 0.33 | 10.53 | 11.22 | 15.0 |
| **iteracion completa** | 1000 | 72.63 | 72.54 | 3.11 | 67.89 | 77.71 | 100.0 |

#### bigblue1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 26.18 | 26.17 | 0.24 | 26.01 | 26.31 | 36.6 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 23.49 | 23.50 | 0.07 | 23.38 | 23.58 | 32.8 |
| densidad | 1000 | 5.87 | 5.85 | 0.17 | 5.78 | 5.95 | 8.2 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 1.04 | 1.04 | 0.01 | 1.03 | 1.05 | 1.5 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.10 | 0.01 | 1.09 | 1.13 | 1.5 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.3 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.14 | 1.12 | 0.03 | 1.11 | 1.18 | 1.6 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 2.24 | 2.24 | 0.09 | 2.18 | 2.32 | 3.1 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 2.48 | 2.45 | 0.24 | 2.40 | 2.53 | 3.5 |
| calibracion de lambda | 1000 | 19.61 | 18.86 | 3.38 | 15.23 | 25.93 | 27.4 |
| HPWL | 1000 | 8.30 | 7.29 | 1.30 | 7.24 | 10.03 | 11.6 |
| combinar + paso + Nesterov | 1000 | 11.62 | 11.54 | 0.23 | 11.49 | 12.21 | 16.2 |
| **iteracion completa** | 1000 | 71.57 | 70.91 | 3.68 | 66.57 | 78.75 | 100.0 |

#### newblue1

Iteraciones muestreadas: 802 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 802 | 43.75 | 44.37 | 3.18 | 35.93 | 47.05 | 43.2 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 802 | 40.68 | 41.35 | 3.19 | 32.89 | 43.97 | 40.2 |
| densidad | 802 | 6.74 | 6.67 | 0.38 | 6.56 | 7.05 | 6.7 |
| &nbsp;&nbsp;&nbsp;reparto de area | 802 | 1.17 | 1.15 | 0.10 | 1.12 | 1.22 | 1.2 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 802 | 1.11 | 1.10 | 0.01 | 1.09 | 1.13 | 1.1 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 802 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.2 |
| &nbsp;&nbsp;&nbsp;gradiente | 802 | 1.50 | 1.47 | 0.22 | 1.40 | 1.55 | 1.5 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 802 | 2.62 | 2.61 | 0.09 | 2.55 | 2.73 | 2.6 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 802 | 2.86 | 2.81 | 0.26 | 2.74 | 3.13 | 2.8 |
| calibracion de lambda | 802 | 29.08 | 29.03 | 3.64 | 23.14 | 34.97 | 28.7 |
| HPWL | 802 | 8.32 | 6.34 | 2.41 | 6.29 | 11.33 | 8.2 |
| combinar + paso + Nesterov | 802 | 13.35 | 13.34 | 0.34 | 13.29 | 13.43 | 13.2 |
| **iteracion completa** | 802 | 101.24 | 102.03 | 6.18 | 89.73 | 110.26 | 100.0 |

## a2_e1_wirelength_4t

#### adaptec1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 9.60 | 9.58 | 0.13 | 9.49 | 9.73 | 20.7 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 7.45 | 7.43 | 0.05 | 7.38 | 7.53 | 16.1 |
| densidad | 1000 | 4.94 | 4.91 | 0.11 | 4.87 | 5.04 | 10.7 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 0.91 | 0.91 | 0.01 | 0.90 | 0.92 | 2.0 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.11 | 0.01 | 1.09 | 1.13 | 2.4 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.5 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 0.87 | 0.86 | 0.01 | 0.86 | 0.88 | 1.9 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 1.74 | 1.73 | 0.05 | 1.70 | 1.80 | 3.8 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 1.91 | 1.89 | 0.13 | 1.85 | 1.97 | 4.1 |
| calibracion de lambda | 1000 | 16.72 | 16.70 | 1.51 | 14.26 | 19.26 | 36.1 |
| HPWL | 1000 | 6.21 | 4.79 | 1.58 | 4.72 | 7.84 | 13.4 |
| combinar + paso + Nesterov | 1000 | 8.90 | 8.73 | 0.25 | 8.66 | 9.23 | 19.2 |
| **iteracion completa** | 1000 | 46.36 | 46.28 | 2.17 | 42.94 | 49.93 | 100.0 |

#### adaptec2

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 13.39 | 13.37 | 0.16 | 13.26 | 13.56 | 22.6 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 10.91 | 10.90 | 0.07 | 10.83 | 11.01 | 18.4 |
| densidad | 1000 | 5.91 | 5.90 | 0.08 | 5.85 | 5.98 | 10.0 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 1.42 | 1.42 | 0.01 | 1.41 | 1.45 | 2.4 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.11 | 0.02 | 1.09 | 1.14 | 1.9 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.4 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.02 | 1.02 | 0.00 | 1.02 | 1.03 | 1.7 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 2.07 | 2.06 | 0.08 | 2.02 | 2.16 | 3.5 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 2.22 | 2.20 | 0.12 | 2.16 | 2.26 | 3.7 |
| calibracion de lambda | 1000 | 21.44 | 21.39 | 2.45 | 17.42 | 25.62 | 36.2 |
| HPWL | 1000 | 7.60 | 6.01 | 1.85 | 5.95 | 10.06 | 12.8 |
| combinar + paso + Nesterov | 1000 | 10.92 | 11.14 | 0.32 | 10.54 | 11.22 | 18.4 |
| **iteracion completa** | 1000 | 59.27 | 59.11 | 3.05 | 54.38 | 64.31 | 100.0 |

#### bigblue1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 13.50 | 13.48 | 0.16 | 13.36 | 13.66 | 23.0 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 10.85 | 10.83 | 0.07 | 10.77 | 10.97 | 18.5 |
| densidad | 1000 | 5.82 | 5.80 | 0.17 | 5.73 | 5.91 | 9.9 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 1.04 | 1.04 | 0.01 | 1.03 | 1.05 | 1.8 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.10 | 1.10 | 0.02 | 1.08 | 1.13 | 1.9 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.4 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.13 | 1.12 | 0.03 | 1.11 | 1.18 | 1.9 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 2.17 | 2.17 | 0.09 | 2.11 | 2.26 | 3.7 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 2.47 | 2.45 | 0.19 | 2.40 | 2.52 | 4.2 |
| calibracion de lambda | 1000 | 19.68 | 18.96 | 3.38 | 15.35 | 25.87 | 33.5 |
| HPWL | 1000 | 8.42 | 7.30 | 1.31 | 7.25 | 9.96 | 14.3 |
| combinar + paso + Nesterov | 1000 | 11.34 | 11.26 | 0.23 | 11.20 | 11.92 | 19.3 |
| **iteracion completa** | 1000 | 58.75 | 58.07 | 3.72 | 53.63 | 65.67 | 100.0 |

#### newblue1

Iteraciones muestreadas: 825 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 825 | 13.75 | 13.70 | 0.20 | 13.55 | 14.12 | 20.0 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 825 | 10.66 | 10.65 | 0.09 | 10.55 | 10.82 | 15.5 |
| densidad | 825 | 7.42 | 6.65 | 1.27 | 6.25 | 9.26 | 10.8 |
| &nbsp;&nbsp;&nbsp;reparto de area | 825 | 1.37 | 1.15 | 0.39 | 0.99 | 1.93 | 2.0 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 825 | 1.11 | 1.10 | 0.02 | 1.09 | 1.13 | 1.6 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 825 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.3 |
| &nbsp;&nbsp;&nbsp;gradiente | 825 | 2.01 | 1.44 | 0.87 | 1.25 | 3.26 | 2.9 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 825 | 2.63 | 2.62 | 0.08 | 2.56 | 2.77 | 3.8 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 825 | 2.83 | 2.78 | 0.16 | 2.73 | 3.12 | 4.1 |
| calibracion de lambda | 825 | 26.28 | 26.13 | 3.01 | 21.08 | 31.22 | 38.2 |
| HPWL | 825 | 7.97 | 6.35 | 2.36 | 6.29 | 11.36 | 11.6 |
| combinar + paso + Nesterov | 825 | 13.32 | 13.31 | 0.34 | 13.26 | 13.40 | 19.4 |
| **iteracion completa** | 825 | 68.73 | 68.69 | 3.90 | 62.46 | 75.12 | 100.0 |

## a2_e2_cpu_paralelo_4t

#### adaptec1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 9.63 | 9.62 | 0.16 | 9.51 | 9.82 | 34.6 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 7.46 | 7.44 | 0.06 | 7.39 | 7.55 | 26.8 |
| densidad | 1000 | 4.94 | 4.91 | 0.15 | 4.87 | 5.05 | 17.8 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 0.91 | 0.91 | 0.01 | 0.90 | 0.92 | 3.3 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.11 | 0.01 | 1.09 | 1.13 | 4.0 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.8 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 0.87 | 0.87 | 0.01 | 0.86 | 0.89 | 3.1 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 1.77 | 1.75 | 0.10 | 1.70 | 1.90 | 6.4 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 1.93 | 1.89 | 0.18 | 1.86 | 2.19 | 6.9 |
| calibracion de lambda | 1000 | 3.41 | 3.09 | 0.66 | 2.96 | 4.25 | 12.3 |
| HPWL | 1000 | 4.69 | 4.67 | 0.17 | 4.64 | 4.72 | 16.9 |
| combinar + paso + Nesterov | 1000 | 5.12 | 4.96 | 0.30 | 4.83 | 5.49 | 18.4 |
| **iteracion completa** | 1000 | 27.81 | 27.64 | 0.79 | 27.04 | 29.16 | 100.0 |

#### adaptec2

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 13.45 | 13.43 | 0.18 | 13.31 | 13.62 | 37.6 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 10.94 | 10.92 | 0.07 | 10.86 | 11.04 | 30.6 |
| densidad | 1000 | 5.92 | 5.90 | 0.16 | 5.84 | 5.97 | 16.6 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 1.42 | 1.41 | 0.01 | 1.41 | 1.43 | 4.0 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.10 | 0.01 | 1.09 | 1.13 | 3.1 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.6 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.02 | 1.02 | 0.00 | 1.02 | 1.03 | 2.9 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 2.12 | 2.10 | 0.11 | 2.04 | 2.24 | 5.9 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 2.21 | 2.19 | 0.20 | 2.15 | 2.25 | 6.2 |
| calibracion de lambda | 1000 | 3.99 | 3.66 | 0.73 | 3.54 | 4.98 | 11.2 |
| HPWL | 1000 | 5.99 | 5.95 | 0.40 | 5.92 | 6.00 | 16.8 |
| combinar + paso + Nesterov | 1000 | 6.38 | 6.59 | 0.38 | 5.94 | 6.70 | 17.9 |
| **iteracion completa** | 1000 | 35.73 | 35.53 | 0.97 | 34.78 | 37.22 | 100.0 |

#### bigblue1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 13.52 | 13.50 | 0.16 | 13.37 | 13.74 | 36.0 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 10.80 | 10.78 | 0.07 | 10.72 | 10.92 | 28.8 |
| densidad | 1000 | 5.86 | 5.84 | 0.15 | 5.76 | 5.96 | 15.6 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 1.04 | 1.04 | 0.01 | 1.03 | 1.05 | 2.8 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.10 | 0.01 | 1.09 | 1.13 | 2.9 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.6 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.13 | 1.12 | 0.03 | 1.11 | 1.18 | 3.0 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 2.27 | 2.26 | 0.08 | 2.18 | 2.40 | 6.0 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 2.48 | 2.45 | 0.18 | 2.40 | 2.64 | 6.6 |
| calibracion de lambda | 1000 | 4.27 | 3.99 | 0.67 | 3.91 | 5.34 | 11.4 |
| HPWL | 1000 | 7.25 | 7.23 | 0.16 | 7.21 | 7.28 | 19.3 |
| combinar + paso + Nesterov | 1000 | 6.67 | 6.58 | 0.32 | 6.50 | 7.27 | 17.8 |
| **iteracion completa** | 1000 | 37.57 | 37.19 | 0.83 | 36.94 | 38.94 | 100.0 |

#### newblue1

Iteraciones muestreadas: 802 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 802 | 13.77 | 13.73 | 0.24 | 13.58 | 14.07 | 34.9 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 802 | 10.70 | 10.69 | 0.08 | 10.59 | 10.83 | 27.1 |
| densidad | 802 | 6.64 | 6.57 | 0.48 | 6.27 | 7.26 | 16.8 |
| &nbsp;&nbsp;&nbsp;reparto de area | 802 | 1.15 | 1.14 | 0.14 | 1.02 | 1.29 | 2.9 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 802 | 1.11 | 1.11 | 0.01 | 1.09 | 1.13 | 2.8 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 802 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.6 |
| &nbsp;&nbsp;&nbsp;gradiente | 802 | 1.50 | 1.46 | 0.30 | 1.29 | 1.74 | 3.8 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 802 | 2.57 | 2.55 | 0.18 | 2.48 | 2.69 | 6.5 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 802 | 2.84 | 2.79 | 0.24 | 2.74 | 3.13 | 7.2 |
| calibracion de lambda | 802 | 4.86 | 4.63 | 0.69 | 4.50 | 6.17 | 12.3 |
| HPWL | 802 | 6.34 | 6.31 | 0.26 | 6.27 | 6.36 | 16.1 |
| combinar + paso + Nesterov | 802 | 7.86 | 7.83 | 0.29 | 7.76 | 7.97 | 19.9 |
| **iteracion completa** | 802 | 39.47 | 39.14 | 1.07 | 38.62 | 41.33 | 100.0 |

## a2_e3_streams_4t

#### adaptec1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 5.56 | 5.47 | 0.38 | 5.20 | 6.13 | 24.6 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 8.50 | 8.47 | 0.13 | 8.33 | 8.70 | 37.7 |
| densidad | 1000 | 1.14 | 1.07 | 0.13 | 1.01 | 1.37 | 5.0 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 7.37 | 7.34 | 0.12 | 7.23 | 7.52 | 32.7 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.13 | 1.11 | 0.04 | 1.10 | 1.23 | 5.0 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.01 | 0.23 | 0.23 | 1.0 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 0.87 | 0.86 | 0.01 | 0.85 | 0.90 | 3.8 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 0.76 | 0.74 | 0.05 | 0.72 | 0.86 | 3.4 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 6.19 | 5.61 | 1.35 | 5.39 | 9.69 | 27.4 |
| calibracion de lambda | 1000 | 3.81 | 3.09 | 1.60 | 2.99 | 7.12 | 16.9 |
| HPWL | 1000 | 6.88 | 6.88 | 0.34 | 6.63 | 7.06 | 30.5 |
| combinar + paso + Nesterov | 1000 | 5.17 | 4.96 | 0.45 | 4.88 | 5.51 | 22.9 |
| **iteracion completa** | 1000 | 22.56 | 21.85 | 1.88 | 21.26 | 26.33 | 100.0 |

#### adaptec2

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 8.38 | 8.30 | 0.56 | 7.85 | 9.04 | 28.7 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 12.29 | 12.25 | 0.16 | 12.09 | 12.54 | 42.1 |
| densidad | 1000 | 1.28 | 1.22 | 0.12 | 1.17 | 1.46 | 4.4 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 11.70 | 11.63 | 0.32 | 11.46 | 11.95 | 40.1 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.25 | 1.26 | 0.06 | 1.11 | 1.32 | 4.3 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.8 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.03 | 1.02 | 0.01 | 1.01 | 1.04 | 3.5 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 0.94 | 0.91 | 0.06 | 0.88 | 1.08 | 3.2 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 7.33 | 6.76 | 1.38 | 6.59 | 11.11 | 25.1 |
| calibracion de lambda | 1000 | 4.34 | 3.67 | 1.67 | 3.56 | 8.04 | 14.9 |
| HPWL | 1000 | 8.78 | 8.76 | 0.57 | 8.38 | 9.05 | 30.1 |
| combinar + paso + Nesterov | 1000 | 6.42 | 6.61 | 0.42 | 5.95 | 6.74 | 22.0 |
| **iteracion completa** | 1000 | 29.20 | 28.61 | 1.88 | 27.79 | 33.05 | 100.0 |

#### bigblue1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 6.29 | 6.24 | 0.41 | 5.82 | 6.80 | 20.4 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 12.29 | 12.22 | 0.20 | 12.03 | 12.57 | 39.8 |
| densidad | 1000 | 1.55 | 1.54 | 0.05 | 1.51 | 1.61 | 5.0 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 11.24 | 11.18 | 0.25 | 10.97 | 11.53 | 36.4 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.26 | 1.28 | 0.05 | 1.19 | 1.31 | 4.1 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.01 | 0.23 | 0.23 | 0.7 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.17 | 1.16 | 0.03 | 1.14 | 1.23 | 3.8 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 1.00 | 0.98 | 0.05 | 0.96 | 1.11 | 3.3 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 8.29 | 7.42 | 1.59 | 7.04 | 11.75 | 26.8 |
| calibracion de lambda | 1000 | 5.39 | 4.04 | 2.17 | 3.89 | 10.16 | 17.4 |
| HPWL | 1000 | 10.87 | 10.91 | 0.37 | 10.52 | 11.09 | 35.2 |
| combinar + paso + Nesterov | 1000 | 6.79 | 6.63 | 0.57 | 6.54 | 7.38 | 22.0 |
| **iteracion completa** | 1000 | 30.88 | 29.55 | 2.46 | 29.02 | 35.73 | 100.0 |

#### newblue1

Iteraciones muestreadas: 875 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 875 | 8.40 | 8.38 | 0.65 | 7.64 | 9.20 | 25.6 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 875 | 12.07 | 12.05 | 0.17 | 11.85 | 12.33 | 36.8 |
| densidad | 875 | 1.71 | 1.69 | 0.07 | 1.66 | 1.84 | 5.2 |
| &nbsp;&nbsp;&nbsp;reparto de area | 875 | 11.15 | 11.13 | 0.19 | 10.89 | 11.43 | 34.0 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 875 | 1.20 | 1.13 | 0.09 | 1.10 | 1.31 | 3.7 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 875 | 0.24 | 0.23 | 0.01 | 0.23 | 0.27 | 0.7 |
| &nbsp;&nbsp;&nbsp;gradiente | 875 | 1.50 | 1.47 | 0.25 | 1.32 | 1.71 | 4.6 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 875 | 1.19 | 1.18 | 0.04 | 1.15 | 1.25 | 3.6 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 875 | 8.74 | 8.18 | 1.36 | 7.64 | 12.20 | 26.6 |
| calibracion de lambda | 875 | 5.69 | 4.72 | 1.81 | 4.48 | 9.29 | 17.3 |
| HPWL | 875 | 9.07 | 9.04 | 0.45 | 8.67 | 9.36 | 27.6 |
| combinar + paso + Nesterov | 875 | 7.95 | 7.90 | 0.38 | 7.81 | 8.03 | 24.2 |
| **iteracion completa** | 875 | 32.81 | 31.98 | 1.93 | 31.16 | 36.34 | 100.0 |

## a2_e4_calidad_4t

#### adaptec1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 5.66 | 5.57 | 0.38 | 5.28 | 6.28 | 24.3 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 8.48 | 8.44 | 0.13 | 8.31 | 8.69 | 36.5 |
| densidad | 1000 | 1.13 | 1.06 | 0.13 | 1.00 | 1.34 | 4.8 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 7.37 | 7.32 | 0.16 | 7.21 | 7.52 | 31.7 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.13 | 1.11 | 0.04 | 1.10 | 1.24 | 4.8 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.01 | 0.23 | 0.23 | 1.0 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 0.86 | 0.86 | 0.01 | 0.85 | 0.89 | 3.7 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 0.75 | 0.72 | 0.06 | 0.70 | 0.91 | 3.2 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 6.11 | 5.52 | 1.35 | 5.38 | 9.67 | 26.3 |
| calibracion de lambda | 1000 | 3.77 | 3.08 | 1.64 | 2.97 | 7.14 | 16.2 |
| HPWL | 1000 | 6.75 | 6.76 | 0.31 | 6.48 | 6.95 | 29.0 |
| combinar + paso + Nesterov | 1000 | 5.95 | 6.09 | 0.47 | 5.56 | 6.21 | 25.6 |
| **iteracion completa** | 1000 | 23.26 | 22.59 | 1.94 | 21.92 | 27.23 | 100.0 |

#### adaptec2

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 8.32 | 8.23 | 0.50 | 7.77 | 8.98 | 27.6 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 12.30 | 12.26 | 0.16 | 12.09 | 12.54 | 40.8 |
| densidad | 1000 | 1.28 | 1.22 | 0.14 | 1.15 | 1.45 | 4.2 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 11.68 | 11.63 | 0.21 | 11.47 | 11.92 | 38.8 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.25 | 1.26 | 0.06 | 1.13 | 1.32 | 4.2 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.8 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.03 | 1.02 | 0.01 | 1.01 | 1.04 | 3.4 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 0.90 | 0.88 | 0.05 | 0.87 | 1.00 | 3.0 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 7.40 | 6.74 | 1.44 | 6.57 | 11.14 | 24.6 |
| calibracion de lambda | 1000 | 4.40 | 3.65 | 1.71 | 3.54 | 8.06 | 14.6 |
| HPWL | 1000 | 8.83 | 8.83 | 0.48 | 8.43 | 9.12 | 29.3 |
| combinar + paso + Nesterov | 1000 | 7.29 | 7.48 | 0.41 | 6.81 | 7.56 | 24.2 |
| **iteracion completa** | 1000 | 30.12 | 29.45 | 1.87 | 28.63 | 34.13 | 100.0 |

#### bigblue1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 6.30 | 6.25 | 0.41 | 5.82 | 6.82 | 19.6 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 12.27 | 12.21 | 0.20 | 12.01 | 12.55 | 38.1 |
| densidad | 1000 | 1.55 | 1.54 | 0.05 | 1.51 | 1.62 | 4.8 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 11.24 | 11.18 | 0.27 | 10.96 | 11.52 | 34.9 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.26 | 1.29 | 0.05 | 1.19 | 1.31 | 3.9 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.7 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.17 | 1.15 | 0.03 | 1.14 | 1.22 | 3.6 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 0.99 | 0.97 | 0.05 | 0.95 | 1.09 | 3.1 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 8.58 | 7.46 | 1.75 | 7.05 | 11.80 | 26.7 |
| calibracion de lambda | 1000 | 5.76 | 4.16 | 2.38 | 3.87 | 10.53 | 17.9 |
| HPWL | 1000 | 10.82 | 10.87 | 0.39 | 10.46 | 11.05 | 33.6 |
| combinar + paso + Nesterov | 1000 | 7.73 | 7.49 | 0.71 | 7.44 | 8.29 | 24.0 |
| **iteracion completa** | 1000 | 32.17 | 30.92 | 2.70 | 29.87 | 37.44 | 100.0 |

#### newblue1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 10.46 | 9.95 | 0.84 | 9.58 | 11.49 | 29.4 |
| &nbsp;&nbsp;&nbsp;kernels WA (GPU) | 1000 | 12.10 | 12.08 | 0.17 | 11.86 | 12.36 | 34.0 |
| densidad | 1000 | 1.73 | 1.71 | 0.11 | 1.68 | 1.79 | 4.9 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 12.04 | 12.04 | 0.18 | 11.79 | 12.27 | 33.8 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.12 | 1.11 | 0.03 | 1.10 | 1.16 | 3.1 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.01 | 0.23 | 0.23 | 0.6 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 3.30 | 3.31 | 0.07 | 3.20 | 3.38 | 9.3 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (GPU) | 1000 | 1.18 | 1.17 | 0.03 | 1.15 | 1.23 | 3.3 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (GPU) | 1000 | 10.04 | 10.39 | 1.28 | 8.77 | 12.20 | 28.2 |
| calibracion de lambda | 1000 | 5.16 | 4.73 | 1.31 | 4.55 | 7.70 | 14.5 |
| HPWL | 1000 | 9.11 | 9.09 | 0.45 | 8.69 | 9.40 | 25.6 |
| combinar + paso + Nesterov | 1000 | 9.15 | 8.95 | 0.44 | 8.91 | 9.76 | 25.7 |
| **iteracion completa** | 1000 | 35.60 | 35.42 | 1.61 | 34.31 | 38.61 | 100.0 |

## Cobertura de las funciones descargadas (linea base: a2_e0_base_gpu_4t)

Porcentaje del tiempo del ciclo que ocupan las funciones, para el criterio de cubrir al menos el 50 %.

| Circuito | wirelength + densidad (%) | + calibracion de lambda (%) | solo lambda (%) |
|---|---|---|---|
| adaptec1 | 44.4 | 73.7 | 29.3 |
| adaptec2 | 45.2 | 74.9 | 29.7 |
| bigblue1 | 44.8 | 72.2 | 27.4 |
| newblue1 | 49.9 | 78.6 | 28.7 |

## Speedup de a2_e1_wirelength_4t contra a2_e0_base_gpu_4t

Cociente de las medias por iteracion (la media es la que define el tiempo total).

| Circuito | Etapa | a2_e0_base_gpu_4t (ms) | a2_e1_wirelength_4t (ms) | Speedup |
|---|---|---|---|---|
| adaptec1 | wirelength | 20.29 | 9.60 | 2.12x |
| adaptec1 | density | 4.94 | 4.94 | 1.00x |
| adaptec1 | lambda | 16.64 | 16.72 | 1.00x |
| adaptec1 | hpwl | 6.05 | 6.21 | 0.97x |
| adaptec1 | update | 8.90 | 8.90 | 1.00x |
| adaptec1 | iteracion completa | 56.83 | 46.36 | 1.23x |
| adaptec2 | wirelength | 26.86 | 13.39 | 2.01x |
| adaptec2 | density | 5.96 | 5.91 | 1.01x |
| adaptec2 | lambda | 21.56 | 21.44 | 1.01x |
| adaptec2 | hpwl | 7.32 | 7.60 | 0.96x |
| adaptec2 | update | 10.93 | 10.92 | 1.00x |
| adaptec2 | iteracion completa | 72.63 | 59.27 | 1.23x |
| bigblue1 | wirelength | 26.18 | 13.50 | 1.94x |
| bigblue1 | density | 5.87 | 5.82 | 1.01x |
| bigblue1 | lambda | 19.61 | 19.68 | 1.00x |
| bigblue1 | hpwl | 8.30 | 8.42 | 0.99x |
| bigblue1 | update | 11.62 | 11.34 | 1.02x |
| bigblue1 | iteracion completa | 71.57 | 58.75 | 1.22x |
| newblue1 | wirelength | 43.75 | 13.75 | 3.18x |
| newblue1 | density | 6.74 | 7.42 | 0.91x |
| newblue1 | lambda | 29.08 | 26.28 | 1.11x |
| newblue1 | hpwl | 8.32 | 7.97 | 1.04x |
| newblue1 | update | 13.35 | 13.32 | 1.00x |
| newblue1 | iteracion completa | 101.24 | 68.73 | 1.47x |

## Speedup de a2_e2_cpu_paralelo_4t contra a2_e0_base_gpu_4t

Cociente de las medias por iteracion (la media es la que define el tiempo total).

| Circuito | Etapa | a2_e0_base_gpu_4t (ms) | a2_e2_cpu_paralelo_4t (ms) | Speedup |
|---|---|---|---|---|
| adaptec1 | wirelength | 20.29 | 9.63 | 2.11x |
| adaptec1 | density | 4.94 | 4.94 | 1.00x |
| adaptec1 | lambda | 16.64 | 3.41 | 4.88x |
| adaptec1 | hpwl | 6.05 | 4.69 | 1.29x |
| adaptec1 | update | 8.90 | 5.12 | 1.74x |
| adaptec1 | iteracion completa | 56.83 | 27.81 | 2.04x |
| adaptec2 | wirelength | 26.86 | 13.45 | 2.00x |
| adaptec2 | density | 5.96 | 5.92 | 1.01x |
| adaptec2 | lambda | 21.56 | 3.99 | 5.40x |
| adaptec2 | hpwl | 7.32 | 5.99 | 1.22x |
| adaptec2 | update | 10.93 | 6.38 | 1.71x |
| adaptec2 | iteracion completa | 72.63 | 35.73 | 2.03x |
| bigblue1 | wirelength | 26.18 | 13.52 | 1.94x |
| bigblue1 | density | 5.87 | 5.86 | 1.00x |
| bigblue1 | lambda | 19.61 | 4.27 | 4.59x |
| bigblue1 | hpwl | 8.30 | 7.25 | 1.14x |
| bigblue1 | update | 11.62 | 6.67 | 1.74x |
| bigblue1 | iteracion completa | 71.57 | 37.57 | 1.90x |
| newblue1 | wirelength | 43.75 | 13.77 | 3.18x |
| newblue1 | density | 6.74 | 6.64 | 1.02x |
| newblue1 | lambda | 29.08 | 4.86 | 5.98x |
| newblue1 | hpwl | 8.32 | 6.34 | 1.31x |
| newblue1 | update | 13.35 | 7.86 | 1.70x |
| newblue1 | iteracion completa | 101.24 | 39.47 | 2.56x |

## Speedup de a2_e3_streams_4t contra a2_e0_base_gpu_4t

Cociente de las medias por iteracion (la media es la que define el tiempo total).

| Circuito | Etapa | a2_e0_base_gpu_4t (ms) | a2_e3_streams_4t (ms) | Speedup |
|---|---|---|---|---|
| adaptec1 | wirelength | 20.29 | 5.56 | 3.65x |
| adaptec1 | density | 4.94 | 1.14 | 4.35x |
| adaptec1 | lambda | 16.64 | 3.81 | 4.36x |
| adaptec1 | hpwl | 6.05 | 6.88 | 0.88x |
| adaptec1 | update | 8.90 | 5.17 | 1.72x |
| adaptec1 | iteracion completa | 56.83 | 22.56 | 2.52x |
| adaptec2 | wirelength | 26.86 | 8.38 | 3.21x |
| adaptec2 | density | 5.96 | 1.28 | 4.65x |
| adaptec2 | lambda | 21.56 | 4.34 | 4.96x |
| adaptec2 | hpwl | 7.32 | 8.78 | 0.83x |
| adaptec2 | update | 10.93 | 6.42 | 1.70x |
| adaptec2 | iteracion completa | 72.63 | 29.20 | 2.49x |
| bigblue1 | wirelength | 26.18 | 6.29 | 4.16x |
| bigblue1 | density | 5.87 | 1.55 | 3.78x |
| bigblue1 | lambda | 19.61 | 5.39 | 3.64x |
| bigblue1 | hpwl | 8.30 | 10.87 | 0.76x |
| bigblue1 | update | 11.62 | 6.79 | 1.71x |
| bigblue1 | iteracion completa | 71.57 | 30.88 | 2.32x |
| newblue1 | wirelength | 43.75 | 8.40 | 5.21x |
| newblue1 | density | 6.74 | 1.71 | 3.95x |
| newblue1 | lambda | 29.08 | 5.69 | 5.11x |
| newblue1 | hpwl | 8.32 | 9.07 | 0.92x |
| newblue1 | update | 13.35 | 7.95 | 1.68x |
| newblue1 | iteracion completa | 101.24 | 32.81 | 3.09x |

## Speedup de a2_e4_calidad_4t contra a2_e0_base_gpu_4t

Cociente de las medias por iteracion (la media es la que define el tiempo total).

| Circuito | Etapa | a2_e0_base_gpu_4t (ms) | a2_e4_calidad_4t (ms) | Speedup |
|---|---|---|---|---|
| adaptec1 | wirelength | 20.29 | 5.66 | 3.58x |
| adaptec1 | density | 4.94 | 1.13 | 4.39x |
| adaptec1 | lambda | 16.64 | 3.77 | 4.41x |
| adaptec1 | hpwl | 6.05 | 6.75 | 0.90x |
| adaptec1 | update | 8.90 | 5.95 | 1.50x |
| adaptec1 | iteracion completa | 56.83 | 23.26 | 2.44x |
| adaptec2 | wirelength | 26.86 | 8.32 | 3.23x |
| adaptec2 | density | 5.96 | 1.28 | 4.66x |
| adaptec2 | lambda | 21.56 | 4.40 | 4.90x |
| adaptec2 | hpwl | 7.32 | 8.83 | 0.83x |
| adaptec2 | update | 10.93 | 7.29 | 1.50x |
| adaptec2 | iteracion completa | 72.63 | 30.12 | 2.41x |
| bigblue1 | wirelength | 26.18 | 6.30 | 4.15x |
| bigblue1 | density | 5.87 | 1.55 | 3.77x |
| bigblue1 | lambda | 19.61 | 5.76 | 3.40x |
| bigblue1 | hpwl | 8.30 | 10.82 | 0.77x |
| bigblue1 | update | 11.62 | 7.73 | 1.50x |
| bigblue1 | iteracion completa | 71.57 | 32.17 | 2.22x |
| newblue1 | wirelength | 43.75 | 10.46 | 4.18x |
| newblue1 | density | 6.74 | 1.73 | 3.89x |
| newblue1 | lambda | 29.08 | 5.16 | 5.64x |
| newblue1 | hpwl | 8.32 | 9.11 | 0.91x |
| newblue1 | update | 13.35 | 9.15 | 1.46x |
| newblue1 | iteracion completa | 101.24 | 35.60 | 2.84x |

## Tiempo total de la corrida: a2_e1_wirelength_4t contra a2_e0_base_gpu_4t

| Circuito | a2_e0_base_gpu_4t (s) | a2_e1_wirelength_4t (s) | Speedup |
|---|---|---|---|
| adaptec1 | 71.3 | 60.9 | 1.17x |
| adaptec2 | 91.3 | 77.9 | 1.17x |
| bigblue1 | 90.3 | 77.6 | 1.16x |
| newblue1 | 113.2 | 93.2 | 1.21x |

## Tiempo total de la corrida: a2_e2_cpu_paralelo_4t contra a2_e0_base_gpu_4t

| Circuito | a2_e0_base_gpu_4t (s) | a2_e2_cpu_paralelo_4t (s) | Speedup |
|---|---|---|---|
| adaptec1 | 71.3 | 42.3 | 1.68x |
| adaptec2 | 91.3 | 54.3 | 1.68x |
| bigblue1 | 90.3 | 56.3 | 1.60x |
| newblue1 | 113.2 | 66.2 | 1.71x |

## Tiempo total de la corrida: a2_e3_streams_4t contra a2_e0_base_gpu_4t

| Circuito | a2_e0_base_gpu_4t (s) | a2_e3_streams_4t (s) | Speedup |
|---|---|---|---|
| adaptec1 | 71.3 | 37.1 | 1.92x |
| adaptec2 | 91.3 | 47.8 | 1.91x |
| bigblue1 | 90.3 | 49.5 | 1.82x |
| newblue1 | 113.2 | 62.0 | 1.83x |

## Tiempo total de la corrida: a2_e4_calidad_4t contra a2_e0_base_gpu_4t

| Circuito | a2_e0_base_gpu_4t (s) | a2_e4_calidad_4t (s) | Speedup |
|---|---|---|---|
| adaptec1 | 71.3 | 37.7 | 1.89x |
| adaptec2 | 91.3 | 48.9 | 1.87x |
| bigblue1 | 90.3 | 50.7 | 1.78x |
| newblue1 | 113.2 | 56.9 | 1.99x |

## Etapas fuera del ciclo y calidad

Una muestra por corrida (del .log). Tiempos en segundos.

| Config | Circuito | parseo | initial_place | preparacion | nesterov | legalizacion | escritura | total | GPU lista | HPWL global | overflow | HPWL legalizado |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| a2_e0_base_gpu_4t | adaptec1 | 3.05 | 6.15 | 1.23 | 56.83 | 3.48 | 0.55 | 71.29 | 1.08 | 9.2139e+07 | 0.290 | 1.5502e+08 |
| a2_e0_base_gpu_4t | adaptec2 | 3.70 | 7.59 | 1.28 | 72.63 | 5.45 | 0.69 | 91.35 | 1.15 | 9.4131e+07 | 0.424 | 2.4099e+08 |
| a2_e0_base_gpu_4t | bigblue1 | 3.90 | 8.25 | 1.28 | 71.57 | 4.59 | 0.75 | 90.34 | 1.16 | 1.1373e+08 | 0.197 | 1.3028e+08 |
| a2_e0_base_gpu_4t | newblue1 | 4.21 | 9.10 | 1.30 | 81.20 | 16.52 | 0.88 | 113.22 | 1.15 | 5.1311e+07 | 0.438 | 1.4738e+09 |
| a2_e1_wirelength_4t | adaptec1 | 3.15 | 6.14 | 1.20 | 46.36 | 3.49 | 0.55 | 60.88 | 1.15 | 9.2131e+07 | 0.290 | 1.5464e+08 |
| a2_e1_wirelength_4t | adaptec2 | 3.69 | 7.58 | 1.25 | 59.27 | 5.45 | 0.69 | 77.94 | 1.18 | 9.4005e+07 | 0.425 | 2.4421e+08 |
| a2_e1_wirelength_4t | bigblue1 | 3.94 | 8.24 | 1.31 | 58.75 | 4.58 | 0.75 | 77.57 | 1.17 | 1.1372e+08 | 0.197 | 1.3020e+08 |
| a2_e1_wirelength_4t | newblue1 | 4.40 | 9.08 | 1.32 | 56.70 | 20.84 | 0.88 | 93.22 | 1.16 | 4.8141e+07 | 0.464 | 1.5286e+09 |
| a2_e2_cpu_paralelo_4t | adaptec1 | 3.07 | 6.16 | 1.23 | 27.81 | 3.49 | 0.55 | 42.31 | 1.16 | 9.2163e+07 | 0.289 | 1.5493e+08 |
| a2_e2_cpu_paralelo_4t | adaptec2 | 3.54 | 7.62 | 1.25 | 35.73 | 5.50 | 0.70 | 54.34 | 1.20 | 9.3988e+07 | 0.425 | 2.4273e+08 |
| a2_e2_cpu_paralelo_4t | bigblue1 | 3.88 | 8.28 | 1.25 | 37.57 | 4.59 | 0.74 | 56.30 | 1.19 | 1.1380e+08 | 0.197 | 1.3013e+08 |
| a2_e2_cpu_paralelo_4t | newblue1 | 4.22 | 9.08 | 1.27 | 31.66 | 19.15 | 0.85 | 66.22 | 1.18 | 4.8842e+07 | 0.463 | 1.4998e+09 |
| a2_e3_streams_4t | adaptec1 | 3.21 | 6.13 | 1.21 | 22.56 | 3.49 | 0.55 | 37.14 | 1.16 | 9.2129e+07 | 0.290 | 1.5435e+08 |
| a2_e3_streams_4t | adaptec2 | 3.67 | 7.58 | 1.22 | 29.20 | 5.46 | 0.68 | 47.80 | 1.19 | 9.4076e+07 | 0.425 | 2.3983e+08 |
| a2_e3_streams_4t | bigblue1 | 3.79 | 8.24 | 1.27 | 30.88 | 4.58 | 0.75 | 49.53 | 1.16 | 1.1374e+08 | 0.197 | 1.2995e+08 |
| a2_e3_streams_4t | newblue1 | 4.31 | 9.09 | 1.30 | 28.71 | 17.71 | 0.86 | 61.98 | 1.17 | 4.7644e+07 | 0.522 | 1.5395e+09 |
| a2_e4_calidad_4t | adaptec1 | 3.05 | 6.14 | 1.20 | 23.27 | 3.47 | 0.59 | 37.71 | 1.15 | 9.1457e+07 | 0.308 | 1.4889e+08 |
| a2_e4_calidad_4t | adaptec2 | 3.73 | 7.63 | 1.26 | 30.12 | 5.47 | 0.71 | 48.91 | 1.19 | 9.3987e+07 | 0.424 | 2.4227e+08 |
| a2_e4_calidad_4t | bigblue1 | 3.77 | 8.20 | 1.23 | 32.17 | 4.56 | 0.78 | 50.71 | 1.19 | 1.1405e+08 | 0.195 | 1.3194e+08 |
| a2_e4_calidad_4t | newblue1 | 4.37 | 9.06 | 1.24 | 35.60 | 5.70 | 0.91 | 56.88 | 1.17 | 1.0685e+08 | 0.164 | 2.1408e+08 |

## Recursos durante cada corrida (tegrastats)

Una muestra por segundo durante toda la corrida (parseo a escritura). CPU es el uso promedio de los cuatro nucleos; la potencia es la de entrada de la placa (POM_5V_IN) y la energia, su suma en el tiempo.

| Config | Circuito | Muestras | CPU media (%) | GPU media (%) | GPU max (%) | RAM de la placa max (MB) | T CPU max (C) | T GPU max (C) | Reloj CPU min (MHz) | Potencia media (W) | Energia (kJ) |
|---|---|---|---|---|---|---|---|---|---|---|---|
| a2_e0_base_gpu_4t | adaptec1 | 71 | 23 | 31 | 99 | 894 | 42.5 | 41.5 | 1479 | - | - |
| a2_e0_base_gpu_4t | adaptec2 | 91 | 22 | 34 | 99 | 904 | 44.0 | 44.0 | 1479 | - | - |
| a2_e0_base_gpu_4t | bigblue1 | 90 | 23 | 28 | 99 | 909 | 46.0 | 45.5 | 1479 | - | - |
| a2_e0_base_gpu_4t | newblue1 | 113 | 21 | 30 | 99 | 924 | 47.0 | 46.5 | 1479 | - | - |
| a2_e1_wirelength_4t | adaptec1 | 60 | 27 | 19 | 67 | 898 | 47.5 | 47.0 | 1479 | - | - |
| a2_e1_wirelength_4t | adaptec2 | 77 | 26 | 19 | 89 | 910 | 48.0 | 47.0 | 1479 | - | - |
| a2_e1_wirelength_4t | bigblue1 | 77 | 27 | 22 | 87 | 914 | 48.5 | 47.5 | 1479 | - | - |
| a2_e1_wirelength_4t | newblue1 | 93 | 26 | 19 | 82 | 930 | 49.0 | 48.0 | 1479 | - | - |
| a2_e2_cpu_paralelo_4t | adaptec1 | 42 | 44 | 24 | 65 | 903 | 50.5 | 49.5 | 1479 | - | - |
| a2_e2_cpu_paralelo_4t | adaptec2 | 54 | 41 | 27 | 86 | 913 | 50.5 | 49.5 | 1479 | - | - |
| a2_e2_cpu_paralelo_4t | bigblue1 | 56 | 42 | 28 | 87 | 918 | 51.0 | 49.5 | 1479 | - | - |
| a2_e2_cpu_paralelo_4t | newblue1 | 66 | 39 | 18 | 86 | 932 | 51.0 | 49.5 | 1479 | - | - |
| a2_e3_streams_4t | adaptec1 | 37 | 57 | 28 | 70 | 908 | 52.5 | 50.5 | 1479 | - | - |
| a2_e3_streams_4t | adaptec2 | 47 | 54 | 32 | 95 | 917 | 52.0 | 50.5 | 1479 | - | - |
| a2_e3_streams_4t | bigblue1 | 49 | 55 | 34 | 95 | 924 | 52.0 | 50.5 | 1479 | - | - |
| a2_e3_streams_4t | newblue1 | 61 | 49 | 22 | 96 | 938 | 52.5 | 50.5 | 1479 | - | - |
| a2_e4_calidad_4t | adaptec1 | 37 | 58 | 30 | 70 | 922 | 50.0 | 48.0 | 1479 | - | - |
| a2_e4_calidad_4t | adaptec2 | 48 | 55 | 35 | 91 | 932 | 51.0 | 49.5 | 1479 | - | - |
| a2_e4_calidad_4t | bigblue1 | 50 | 55 | 33 | 95 | 937 | 52.0 | 50.5 | 1479 | - | - |
| a2_e4_calidad_4t | newblue1 | 56 | 56 | 34 | 99 | 952 | 52.5 | 50.5 | 1479 | - | - |

