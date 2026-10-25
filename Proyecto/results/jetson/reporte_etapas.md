# Perfilado por etapa

Tiempos de pared por iteracion del ciclo de Nesterov (una fila del CSV por iteracion). En la version GPU las partes de cada etapa se miden con eventos de CUDA y las etapas completas con el reloj del host, que incluye el costo de lanzar y esperar.

## cpu_base_4t

#### adaptec1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 104.04 | 103.01 | 5.07 | 102.69 | 107.97 | 19.1 |
| densidad | 1000 | 175.78 | 175.95 | 4.95 | 166.96 | 183.22 | 32.2 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 13.64 | 13.55 | 0.53 | 13.32 | 13.92 | 2.5 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 154.11 | 154.34 | 4.87 | 145.41 | 161.22 | 28.3 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 1.28 | 1.28 | 0.02 | 1.26 | 1.31 | 0.2 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 6.74 | 6.68 | 0.41 | 6.60 | 6.83 | 1.2 |
| calibracion de lambda | 1000 | 251.58 | 250.74 | 4.25 | 247.17 | 259.79 | 46.1 |
| HPWL | 1000 | 5.01 | 4.80 | 0.79 | 4.69 | 7.81 | 0.9 |
| combinar + paso + Nesterov | 1000 | 8.79 | 8.69 | 0.29 | 8.35 | 9.20 | 1.6 |
| **iteracion completa** | 1000 | 545.20 | 544.57 | 8.40 | 533.81 | 557.74 | 100.0 |

#### adaptec2

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 123.66 | 122.82 | 4.50 | 122.46 | 126.80 | 19.6 |
| densidad | 1000 | 178.90 | 179.02 | 4.57 | 171.01 | 185.85 | 28.3 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 15.81 | 15.76 | 0.56 | 15.54 | 15.93 | 2.5 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 153.76 | 153.89 | 4.54 | 145.96 | 160.71 | 24.4 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 1.31 | 1.31 | 0.03 | 1.28 | 1.33 | 0.2 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 8.02 | 7.98 | 0.36 | 7.87 | 8.15 | 1.3 |
| calibracion de lambda | 1000 | 311.32 | 311.03 | 2.88 | 307.37 | 318.14 | 49.3 |
| HPWL | 1000 | 6.70 | 5.98 | 1.49 | 5.94 | 10.12 | 1.1 |
| combinar + paso + Nesterov | 1000 | 10.75 | 10.55 | 0.37 | 9.97 | 11.16 | 1.7 |
| **iteracion completa** | 1000 | 631.32 | 630.89 | 6.97 | 621.78 | 641.19 | 100.0 |

#### bigblue1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 147.86 | 147.69 | 1.70 | 146.96 | 148.26 | 22.4 |
| densidad | 1000 | 180.76 | 180.82 | 4.98 | 172.77 | 187.72 | 27.4 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 16.27 | 16.15 | 0.72 | 15.94 | 16.47 | 2.5 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 154.36 | 154.45 | 4.81 | 146.46 | 161.16 | 23.4 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 1.22 | 1.21 | 0.03 | 1.19 | 1.28 | 0.2 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 8.91 | 8.74 | 0.69 | 8.68 | 9.11 | 1.3 |
| calibracion de lambda | 1000 | 312.75 | 307.47 | 10.74 | 303.09 | 330.37 | 47.4 |
| HPWL | 1000 | 7.43 | 7.25 | 0.69 | 7.17 | 9.81 | 1.1 |
| combinar + paso + Nesterov | 1000 | 11.39 | 11.44 | 0.36 | 10.83 | 12.09 | 1.7 |
| **iteracion completa** | 1000 | 660.18 | 656.24 | 11.82 | 645.63 | 680.54 | 100.0 |

#### newblue1

Iteraciones muestreadas: 817 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 817 | 143.20 | 142.45 | 4.41 | 142.10 | 146.55 | 18.8 |
| densidad | 817 | 177.73 | 176.97 | 4.19 | 172.47 | 186.65 | 23.4 |
| &nbsp;&nbsp;&nbsp;reparto de area | 817 | 18.21 | 18.23 | 0.75 | 16.85 | 18.99 | 2.4 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 817 | 147.46 | 146.63 | 4.07 | 142.51 | 156.50 | 19.4 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 817 | 1.20 | 1.20 | 0.04 | 1.15 | 1.23 | 0.2 |
| &nbsp;&nbsp;&nbsp;gradiente | 817 | 10.86 | 10.86 | 0.61 | 10.26 | 11.12 | 1.4 |
| calibracion de lambda | 817 | 419.29 | 422.79 | 14.19 | 392.30 | 437.05 | 55.1 |
| HPWL | 817 | 7.08 | 6.35 | 1.70 | 6.29 | 11.21 | 0.9 |
| combinar + paso + Nesterov | 817 | 13.22 | 13.32 | 0.42 | 12.76 | 13.41 | 1.7 |
| **iteracion completa** | 817 | 760.53 | 763.95 | 16.07 | 731.22 | 781.09 | 100.0 |

## cpu_quickselect_4t

#### adaptec1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 103.51 | 103.10 | 1.63 | 102.97 | 106.82 | 33.4 |
| densidad | 1000 | 176.00 | 176.24 | 4.73 | 167.57 | 183.03 | 56.8 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 13.64 | 13.53 | 0.55 | 13.38 | 13.96 | 4.4 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 154.32 | 154.63 | 4.68 | 145.94 | 161.22 | 49.8 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 1.29 | 1.28 | 0.03 | 1.26 | 1.33 | 0.4 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 6.75 | 6.67 | 0.41 | 6.62 | 6.88 | 2.2 |
| calibracion de lambda | 1000 | 16.71 | 16.65 | 1.50 | 14.21 | 19.11 | 5.4 |
| HPWL | 1000 | 4.84 | 4.79 | 0.33 | 4.77 | 4.84 | 1.6 |
| combinar + paso + Nesterov | 1000 | 8.90 | 8.74 | 0.26 | 8.63 | 9.25 | 2.9 |
| **iteracion completa** | 1000 | 309.97 | 310.06 | 5.23 | 301.28 | 317.76 | 100.0 |

#### adaptec2

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 122.80 | 122.54 | 1.35 | 122.16 | 125.64 | 36.1 |
| densidad | 1000 | 178.70 | 178.84 | 4.72 | 170.74 | 185.89 | 52.6 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 15.78 | 15.75 | 0.36 | 15.56 | 15.98 | 4.6 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 153.62 | 153.76 | 4.71 | 145.55 | 160.66 | 45.2 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 1.30 | 1.30 | 0.01 | 1.28 | 1.33 | 0.4 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 8.01 | 7.95 | 0.47 | 7.86 | 8.14 | 2.4 |
| calibracion de lambda | 1000 | 21.67 | 21.69 | 2.57 | 17.44 | 26.11 | 6.4 |
| HPWL | 1000 | 6.02 | 5.98 | 0.34 | 5.92 | 6.03 | 1.8 |
| combinar + paso + Nesterov | 1000 | 10.62 | 10.58 | 0.42 | 10.01 | 11.19 | 3.1 |
| **iteracion completa** | 1000 | 339.82 | 339.90 | 5.58 | 330.51 | 348.82 | 100.0 |

#### bigblue1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 147.67 | 147.54 | 0.64 | 147.30 | 148.17 | 40.4 |
| densidad | 1000 | 179.98 | 180.02 | 4.30 | 172.62 | 186.52 | 49.2 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 16.26 | 16.19 | 0.52 | 15.96 | 16.48 | 4.4 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 153.60 | 153.62 | 4.23 | 146.23 | 159.68 | 42.0 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 1.22 | 1.21 | 0.04 | 1.19 | 1.29 | 0.3 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 8.89 | 8.72 | 0.61 | 8.64 | 9.10 | 2.4 |
| calibracion de lambda | 1000 | 19.50 | 18.75 | 3.39 | 15.04 | 25.83 | 5.3 |
| HPWL | 1000 | 7.32 | 7.29 | 0.24 | 7.25 | 7.34 | 2.0 |
| combinar + paso + Nesterov | 1000 | 11.29 | 11.23 | 0.21 | 11.09 | 11.83 | 3.1 |
| **iteracion completa** | 1000 | 365.76 | 365.62 | 5.72 | 356.57 | 375.29 | 100.0 |

#### newblue1

Iteraciones muestreadas: 802 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 802 | 142.67 | 142.25 | 1.72 | 141.95 | 146.03 | 38.3 |
| densidad | 802 | 181.74 | 181.49 | 5.38 | 173.73 | 190.48 | 48.8 |
| &nbsp;&nbsp;&nbsp;reparto de area | 802 | 17.96 | 18.17 | 0.99 | 15.77 | 19.06 | 4.8 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 802 | 151.91 | 151.78 | 5.36 | 143.82 | 160.57 | 40.8 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 802 | 1.19 | 1.19 | 0.03 | 1.15 | 1.22 | 0.3 |
| &nbsp;&nbsp;&nbsp;gradiente | 802 | 10.68 | 10.81 | 0.63 | 9.90 | 10.96 | 2.9 |
| calibracion de lambda | 802 | 28.98 | 29.08 | 3.67 | 23.10 | 34.90 | 7.8 |
| HPWL | 802 | 6.33 | 6.28 | 0.28 | 6.23 | 6.41 | 1.7 |
| combinar + paso + Nesterov | 802 | 12.98 | 12.89 | 0.41 | 12.70 | 13.42 | 3.5 |
| **iteracion completa** | 802 | 372.70 | 372.84 | 7.26 | 361.59 | 383.83 | 100.0 |

## gpu_4t

#### adaptec1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 20.32 | 20.30 | 0.10 | 20.24 | 20.44 | 35.5 |
| &nbsp;&nbsp;&nbsp;kernel WA (GPU) | 1000 | 18.16 | 18.16 | 0.03 | 18.13 | 18.21 | 31.7 |
| densidad | 1000 | 4.94 | 4.92 | 0.09 | 4.88 | 5.04 | 8.6 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 0.91 | 0.90 | 0.01 | 0.90 | 0.91 | 1.6 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.11 | 0.01 | 1.09 | 1.13 | 1.9 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.4 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 0.87 | 0.87 | 0.01 | 0.86 | 0.88 | 1.5 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (dentro de wirelength y densidad) | 1000 | 1.74 | 1.71 | 0.06 | 1.69 | 1.84 | 3.0 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (dentro de wirelength y densidad) | 1000 | 1.92 | 1.89 | 0.11 | 1.86 | 1.97 | 3.3 |
| calibracion de lambda | 1000 | 16.66 | 16.72 | 1.51 | 14.12 | 19.05 | 29.1 |
| HPWL | 1000 | 6.46 | 7.48 | 1.57 | 4.77 | 7.88 | 11.3 |
| combinar + paso + Nesterov | 1000 | 8.87 | 8.70 | 0.25 | 8.64 | 9.21 | 15.5 |
| **iteracion completa** | 1000 | 57.25 | 57.30 | 2.25 | 53.49 | 60.74 | 100.0 |

#### adaptec2

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 26.91 | 26.90 | 0.10 | 26.84 | 27.05 | 36.9 |
| &nbsp;&nbsp;&nbsp;kernel WA (GPU) | 1000 | 24.45 | 24.44 | 0.03 | 24.42 | 24.50 | 33.5 |
| densidad | 1000 | 5.93 | 5.92 | 0.08 | 5.87 | 6.00 | 8.1 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 1.42 | 1.42 | 0.01 | 1.41 | 1.46 | 1.9 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.11 | 1.10 | 0.01 | 1.09 | 1.13 | 1.5 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.3 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.03 | 1.03 | 0.01 | 1.02 | 1.04 | 1.4 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (dentro de wirelength y densidad) | 1000 | 2.07 | 2.07 | 0.06 | 2.02 | 2.16 | 2.8 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (dentro de wirelength y densidad) | 1000 | 2.22 | 2.20 | 0.09 | 2.16 | 2.27 | 3.0 |
| calibracion de lambda | 1000 | 21.32 | 21.32 | 2.55 | 17.14 | 25.64 | 29.2 |
| HPWL | 1000 | 7.88 | 9.17 | 1.81 | 6.03 | 10.18 | 10.8 |
| combinar + paso + Nesterov | 1000 | 10.84 | 11.06 | 0.33 | 10.44 | 11.15 | 14.9 |
| **iteracion completa** | 1000 | 72.88 | 72.83 | 3.34 | 67.47 | 78.61 | 100.0 |

#### bigblue1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 26.18 | 26.17 | 0.12 | 26.05 | 26.34 | 36.5 |
| &nbsp;&nbsp;&nbsp;kernel WA (GPU) | 1000 | 23.51 | 23.53 | 0.07 | 23.40 | 23.59 | 32.8 |
| densidad | 1000 | 5.86 | 5.85 | 0.11 | 5.78 | 5.94 | 8.2 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 1.04 | 1.04 | 0.01 | 1.03 | 1.05 | 1.5 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 1.10 | 1.10 | 0.01 | 1.09 | 1.13 | 1.5 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.3 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 1.14 | 1.12 | 0.03 | 1.11 | 1.18 | 1.6 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (dentro de wirelength y densidad) | 1000 | 2.24 | 2.23 | 0.09 | 2.18 | 2.32 | 3.1 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (dentro de wirelength y densidad) | 1000 | 2.46 | 2.44 | 0.11 | 2.40 | 2.52 | 3.4 |
| calibracion de lambda | 1000 | 19.41 | 18.68 | 3.34 | 15.23 | 25.71 | 27.1 |
| HPWL | 1000 | 8.67 | 9.26 | 1.29 | 7.28 | 10.05 | 12.1 |
| combinar + paso + Nesterov | 1000 | 11.62 | 11.53 | 0.23 | 11.48 | 12.20 | 16.2 |
| **iteracion completa** | 1000 | 71.74 | 70.97 | 3.67 | 66.75 | 78.62 | 100.0 |

#### newblue1

Iteraciones muestreadas: 808 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 808 | 43.62 | 45.29 | 3.74 | 35.87 | 47.39 | 43.0 |
| &nbsp;&nbsp;&nbsp;kernel WA (GPU) | 808 | 40.57 | 42.27 | 3.75 | 32.86 | 44.37 | 40.0 |
| densidad | 808 | 6.69 | 6.61 | 0.39 | 6.48 | 7.16 | 6.6 |
| &nbsp;&nbsp;&nbsp;reparto de area | 808 | 1.17 | 1.15 | 0.12 | 1.10 | 1.27 | 1.2 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 808 | 1.10 | 1.10 | 0.01 | 1.09 | 1.12 | 1.1 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 808 | 0.23 | 0.23 | 0.00 | 0.23 | 0.23 | 0.2 |
| &nbsp;&nbsp;&nbsp;gradiente | 808 | 1.52 | 1.47 | 0.26 | 1.39 | 1.70 | 1.5 |
| &nbsp;&nbsp;&nbsp;copias host->GPU (dentro de wirelength y densidad) | 808 | 2.54 | 2.53 | 0.08 | 2.47 | 2.63 | 2.5 |
| &nbsp;&nbsp;&nbsp;copias GPU->host (dentro de wirelength y densidad) | 808 | 2.85 | 2.82 | 0.14 | 2.76 | 3.14 | 2.8 |
| calibracion de lambda | 808 | 28.78 | 29.02 | 3.79 | 22.51 | 34.66 | 28.4 |
| HPWL | 808 | 8.79 | 6.42 | 2.60 | 6.29 | 11.39 | 8.7 |
| combinar + paso + Nesterov | 808 | 13.47 | 13.44 | 0.37 | 13.38 | 13.62 | 13.3 |
| **iteracion completa** | 808 | 101.36 | 102.19 | 6.97 | 87.82 | 110.96 | 100.0 |

## cpu_base_1t

#### adaptec1

Iteraciones muestreadas: 1000 (una muestra por etapa en cada una).

| Etapa | Muestras | Media (ms) | Mediana (ms) | Desv. (ms) | p5 (ms) | p95 (ms) | % de la iteracion |
|---|---|---|---|---|---|---|---|
| wirelength | 1000 | 397.10 | 397.09 | 0.28 | 396.66 | 397.55 | 37.4 |
| densidad | 1000 | 389.63 | 389.46 | 0.89 | 388.50 | 391.17 | 36.7 |
| &nbsp;&nbsp;&nbsp;reparto de area | 1000 | 45.16 | 45.11 | 0.33 | 44.78 | 45.76 | 4.3 |
| &nbsp;&nbsp;&nbsp;Poisson (DCT ida + vuelta) | 1000 | 317.94 | 317.67 | 0.76 | 317.29 | 319.30 | 30.0 |
| &nbsp;&nbsp;&nbsp;campo + sumas | 1000 | 1.28 | 1.28 | 0.02 | 1.25 | 1.32 | 0.1 |
| &nbsp;&nbsp;&nbsp;gradiente | 1000 | 25.24 | 25.24 | 0.15 | 25.05 | 25.52 | 2.4 |
| calibracion de lambda | 1000 | 251.05 | 250.19 | 3.75 | 246.35 | 257.99 | 23.7 |
| HPWL | 1000 | 14.16 | 14.20 | 0.11 | 14.03 | 14.28 | 1.3 |
| combinar + paso + Nesterov | 1000 | 8.63 | 8.61 | 0.30 | 8.22 | 9.12 | 0.8 |
| **iteracion completa** | 1000 | 1060.56 | 1059.89 | 4.30 | 1054.76 | 1068.64 | 100.0 |

## Cobertura de las funciones descargadas (linea base: cpu_base_4t)

Porcentaje del tiempo del ciclo que ocupan las funciones, para el criterio de cubrir al menos el 50 %.

| Circuito | wirelength + densidad (%) | + calibracion de lambda (%) | solo lambda (%) |
|---|---|---|---|
| adaptec1 | 51.3 | 97.5 | 46.1 |
| adaptec2 | 47.9 | 97.2 | 49.3 |
| bigblue1 | 49.8 | 97.1 | 47.4 |
| newblue1 | 42.2 | 97.3 | 55.1 |

## Speedup de cpu_quickselect_4t contra cpu_base_4t

Cociente de las medias por iteracion (la media es la que define el tiempo total).

| Circuito | Etapa | cpu_base_4t (ms) | cpu_quickselect_4t (ms) | Speedup |
|---|---|---|---|---|
| adaptec1 | wirelength | 104.04 | 103.51 | 1.01x |
| adaptec1 | density | 175.78 | 176.00 | 1.00x |
| adaptec1 | lambda | 251.58 | 16.71 | 15.05x |
| adaptec1 | hpwl | 5.01 | 4.84 | 1.04x |
| adaptec1 | update | 8.79 | 8.90 | 0.99x |
| adaptec1 | iteracion completa | 545.20 | 309.97 | 1.76x |
| adaptec2 | wirelength | 123.66 | 122.80 | 1.01x |
| adaptec2 | density | 178.90 | 178.70 | 1.00x |
| adaptec2 | lambda | 311.32 | 21.67 | 14.37x |
| adaptec2 | hpwl | 6.70 | 6.02 | 1.11x |
| adaptec2 | update | 10.75 | 10.62 | 1.01x |
| adaptec2 | iteracion completa | 631.32 | 339.82 | 1.86x |
| bigblue1 | wirelength | 147.86 | 147.67 | 1.00x |
| bigblue1 | density | 180.76 | 179.98 | 1.00x |
| bigblue1 | lambda | 312.75 | 19.50 | 16.04x |
| bigblue1 | hpwl | 7.43 | 7.32 | 1.02x |
| bigblue1 | update | 11.39 | 11.29 | 1.01x |
| bigblue1 | iteracion completa | 660.18 | 365.76 | 1.80x |
| newblue1 | wirelength | 143.20 | 142.67 | 1.00x |
| newblue1 | density | 177.73 | 181.74 | 0.98x |
| newblue1 | lambda | 419.29 | 28.98 | 14.47x |
| newblue1 | hpwl | 7.08 | 6.33 | 1.12x |
| newblue1 | update | 13.22 | 12.98 | 1.02x |
| newblue1 | iteracion completa | 760.53 | 372.70 | 2.04x |

## Speedup de gpu_4t contra cpu_base_4t

Cociente de las medias por iteracion (la media es la que define el tiempo total).

| Circuito | Etapa | cpu_base_4t (ms) | gpu_4t (ms) | Speedup |
|---|---|---|---|---|
| adaptec1 | wirelength | 104.04 | 20.32 | 5.12x |
| adaptec1 | density | 175.78 | 4.94 | 35.58x |
| adaptec1 | lambda | 251.58 | 16.66 | 15.10x |
| adaptec1 | hpwl | 5.01 | 6.46 | 0.78x |
| adaptec1 | update | 8.79 | 8.87 | 0.99x |
| adaptec1 | iteracion completa | 545.20 | 57.25 | 9.52x |
| adaptec2 | wirelength | 123.66 | 26.91 | 4.60x |
| adaptec2 | density | 178.90 | 5.93 | 30.14x |
| adaptec2 | lambda | 311.32 | 21.32 | 14.60x |
| adaptec2 | hpwl | 6.70 | 7.88 | 0.85x |
| adaptec2 | update | 10.75 | 10.84 | 0.99x |
| adaptec2 | iteracion completa | 631.32 | 72.88 | 8.66x |
| bigblue1 | wirelength | 147.86 | 26.18 | 5.65x |
| bigblue1 | density | 180.76 | 5.86 | 30.84x |
| bigblue1 | lambda | 312.75 | 19.41 | 16.11x |
| bigblue1 | hpwl | 7.43 | 8.67 | 0.86x |
| bigblue1 | update | 11.39 | 11.62 | 0.98x |
| bigblue1 | iteracion completa | 660.18 | 71.74 | 9.20x |
| newblue1 | wirelength | 143.20 | 43.62 | 3.28x |
| newblue1 | density | 177.73 | 6.69 | 26.55x |
| newblue1 | lambda | 419.29 | 28.78 | 14.57x |
| newblue1 | hpwl | 7.08 | 8.79 | 0.81x |
| newblue1 | update | 13.22 | 13.47 | 0.98x |
| newblue1 | iteracion completa | 760.53 | 101.36 | 7.50x |

## Speedup de cpu_base_1t contra cpu_base_4t

Cociente de las medias por iteracion (la media es la que define el tiempo total).

| Circuito | Etapa | cpu_base_4t (ms) | cpu_base_1t (ms) | Speedup |
|---|---|---|---|---|
| adaptec1 | wirelength | 104.04 | 397.10 | 0.26x |
| adaptec1 | density | 175.78 | 389.63 | 0.45x |
| adaptec1 | lambda | 251.58 | 251.05 | 1.00x |
| adaptec1 | hpwl | 5.01 | 14.16 | 0.35x |
| adaptec1 | update | 8.79 | 8.63 | 1.02x |
| adaptec1 | iteracion completa | 545.20 | 1060.56 | 0.51x |

## Tiempo total de la corrida: cpu_quickselect_4t contra cpu_base_4t

| Circuito | cpu_base_4t (s) | cpu_quickselect_4t (s) | Speedup |
|---|---|---|---|
| adaptec1 | 559.3 | 323.8 | 1.73x |
| adaptec2 | 649.6 | 357.8 | 1.82x |
| bigblue1 | 678.7 | 384.0 | 1.77x |
| newblue1 | 661.9 | 335.9 | 1.97x |

## Tiempo total de la corrida: gpu_4t contra cpu_base_4t

| Circuito | cpu_base_4t (s) | gpu_4t (s) | Speedup |
|---|---|---|---|
| adaptec1 | 559.3 | 71.9 | 7.78x |
| adaptec2 | 649.6 | 91.6 | 7.09x |
| bigblue1 | 678.7 | 90.5 | 7.50x |
| newblue1 | 661.9 | 115.2 | 5.74x |

## Tiempo total de la corrida: cpu_base_1t contra cpu_base_4t

| Circuito | cpu_base_4t (s) | cpu_base_1t (s) | Speedup |
|---|---|---|---|
| adaptec1 | 559.3 | 1075.7 | 0.52x |

## Etapas fuera del ciclo y calidad

Una muestra por corrida (del .log). Tiempos en segundos.

| Config | Circuito | parseo | initial_place | preparacion | nesterov | legalizacion | escritura | total | GPU lista | HPWL global | overflow | HPWL legalizado |
|---|---|---|---|---|---|---|---|---|---|---|---|---|
| cpu_base_4t | adaptec1 | 3.10 | 6.14 | 0.86 | 545.20 | 3.46 | 0.57 | 559.33 | - | 9.2145e+07 | 0.289 | 1.5485e+08 |
| cpu_base_4t | adaptec2 | 3.64 | 7.59 | 0.98 | 631.33 | 5.41 | 0.68 | 649.62 | - | 9.3993e+07 | 0.425 | 2.4332e+08 |
| cpu_base_4t | bigblue1 | 3.90 | 8.26 | 1.05 | 660.19 | 4.52 | 0.72 | 678.65 | - | 1.1379e+08 | 0.197 | 1.3034e+08 |
| cpu_base_4t | newblue1 | 4.29 | 9.07 | 1.08 | 621.35 | 25.21 | 0.89 | 661.90 | - | 5.2791e+07 | 0.447 | 1.5579e+09 |
| cpu_quickselect_4t | adaptec1 | 3.02 | 6.15 | 0.59 | 309.97 | 3.49 | 0.56 | 323.77 | - | 9.2150e+07 | 0.289 | 1.5486e+08 |
| cpu_quickselect_4t | adaptec2 | 3.55 | 7.65 | 0.63 | 339.82 | 5.41 | 0.71 | 357.77 | - | 9.3991e+07 | 0.426 | 2.4614e+08 |
| cpu_quickselect_4t | bigblue1 | 3.88 | 8.28 | 0.77 | 365.76 | 4.58 | 0.76 | 384.02 | - | 1.1378e+08 | 0.197 | 1.3034e+08 |
| cpu_quickselect_4t | newblue1 | 4.27 | 9.11 | 0.69 | 298.90 | 22.06 | 0.90 | 335.93 | - | 4.9557e+07 | 0.495 | 1.4861e+09 |
| gpu_4t | adaptec1 | 3.15 | 6.15 | 1.30 | 57.25 | 3.48 | 0.56 | 71.89 | 1.15 | 9.2154e+07 | 0.289 | 1.5472e+08 |
| gpu_4t | adaptec2 | 3.67 | 7.60 | 1.32 | 72.88 | 5.45 | 0.72 | 91.64 | 1.15 | 9.4084e+07 | 0.425 | 2.4144e+08 |
| gpu_4t | bigblue1 | 3.82 | 8.27 | 1.29 | 71.74 | 4.60 | 0.76 | 90.49 | 1.13 | 1.1379e+08 | 0.197 | 1.3032e+08 |
| gpu_4t | newblue1 | 4.38 | 9.09 | 1.22 | 81.90 | 17.75 | 0.91 | 115.24 | 1.10 | 5.1489e+07 | 0.493 | 1.5387e+09 |
| cpu_base_1t | adaptec1 | 3.04 | 6.13 | 1.91 | 1060.56 | 3.48 | 0.55 | 1075.67 | - | 9.2137e+07 | 0.290 | 1.5523e+08 |

## Recursos durante cada corrida (tegrastats)

Una muestra por segundo durante toda la corrida (parseo a escritura). CPU es el uso promedio de los cuatro nucleos; la potencia es la de entrada de la placa (POM_5V_IN) y la energia, su suma en el tiempo.

| Config | Circuito | Muestras | CPU media (%) | GPU media (%) | GPU max (%) | RAM de la placa max (MB) | T CPU max (C) | T GPU max (C) | Reloj CPU min (MHz) | Potencia media (W) | Energia (kJ) |
|---|---|---|---|---|---|---|---|---|---|---|---|
| cpu_base_4t | adaptec1 | 555 | 62 | 0 | 0 | 974 | 50.0 | 47.0 | 1479 | 3.56 | 1.98 |
| cpu_base_4t | adaptec2 | 645 | 59 | 0 | 0 | 984 | 52.0 | 49.0 | 1479 | 3.58 | 2.31 |
| cpu_base_4t | bigblue1 | 674 | 58 | 0 | 0 | 992 | 53.0 | 50.5 | 1479 | 3.63 | 2.44 |
| cpu_base_4t | newblue1 | 657 | 54 | 0 | 0 | 1003 | 53.0 | 50.0 | 1479 | 3.49 | 2.29 |
| cpu_quickselect_4t | adaptec1 | 321 | 89 | 0 | 0 | 977 | 56.0 | 52.5 | 1479 | 4.36 | 1.40 |
| cpu_quickselect_4t | adaptec2 | 355 | 86 | 0 | 0 | 987 | 57.0 | 53.5 | 1479 | 4.35 | 1.55 |
| cpu_quickselect_4t | bigblue1 | 381 | 83 | 0 | 0 | 992 | 58.0 | 54.5 | 1479 | 4.32 | 1.65 |
| cpu_quickselect_4t | newblue1 | 333 | 81 | 0 | 0 | 1003 | 58.5 | 54.5 | 1479 | 4.32 | 1.44 |
| gpu_4t | adaptec1 | 71 | 23 | 27 | 98 | 1095 | 49.5 | 49.5 | 1479 | 3.98 | 0.28 |
| gpu_4t | adaptec2 | 91 | 22 | 33 | 99 | 1105 | 49.5 | 50.0 | 1479 | 3.98 | 0.36 |
| gpu_4t | bigblue1 | 90 | 23 | 30 | 99 | 1111 | 49.5 | 50.0 | 1479 | 4.03 | 0.36 |
| gpu_4t | newblue1 | 114 | 21 | 32 | 99 | 1116 | 49.5 | 49.5 | 1479 | 3.69 | 0.42 |
| cpu_base_1t | adaptec1 | 1069 | 25 | 0 | 0 | 973 | 45.5 | 43.0 | 1479 | 2.75 | 2.94 |

