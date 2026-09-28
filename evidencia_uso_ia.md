# Evidencia de uso de IA — Prototipo en CPU

Resumen de una sesión de trabajo con Claude Code sobre el ubicador analítico de P&R.
Todas las decisiones de diseño, alcance y dirección fueron tomadas por el estudiante;
la IA investigó, propuso hipótesis concretas, implementó cambios puntuales y validó
resultados contra datos reales, siempre reportando antes de avanzar al siguiente paso.

## 1. Diagnóstico de dos fallas de convergencia (`bigblue1` y `newblue1`)

Punto de partida: dos benchmarks reales daban resultados muy malos tras legalizar
(`bigblue1`: HPWL 8.3e8, 9.3x peor que RePlAce; `newblue1`: HPWL explotando durante
la optimización). Se pidió investigar la causa raíz, no solo "arreglar el síntoma".

- Se instrumentó temporalmente el valor de `lambda` (peso de la densidad frente al
  wirelength) iteración por iteración, confirmando que en `bigblue1` la calibración
  automática de `lambda` **bajaba** en vez de subir mientras el overflow seguía alto
  — la fórmula (mediana de gradientes) no tenía forma de notar que hacía falta más
  peso para la densidad.
- Se implementó un escalado activo de `lambda` atado al overflow (si no mejora en
  una ventana de iteraciones, se sube el peso a la fuerza). Un primer intento con
  ventana de 100 iteraciones rompió los benchmarks que ya funcionaban bien
  (`adaptec1`, `adaptec2`) por activarse antes de tiempo; se ajustó a 300
  iteraciones tras comparar las trayectorias de convergencia de cada circuito.
- Resultado verificado: `bigblue1` pasó de HPWL 8.3e8 a 1.3e8 (1.46x RePlAce en vez
  de 9.3x), sin afectar los benchmarks que ya iban bien.
- `newblue1` quedó documentado como limitación conocida: su problema (el overflow
  baja demasiado rápido, no demasiado lento) es el caso contrario al de `bigblue1`
  y el mecanismo implementado no lo cubre; se decidió explícitamente no seguir
  ajustando el algoritmo para no invertir más tiempo del que el curso justifica.

## 2. No determinismo por paralelismo (OpenMP)

Se detectó que dos corridas del mismo benchmark podían dar HPWL legalizado muy
distinto (1.59e8 vs. 5.96e8 para `adaptec1`, mismo binario). Se aisló la causa
corriendo el mismo caso con `OMP_NUM_THREADS=1` dos veces (resultado idéntico bit a
bit) contra corridas normales multi-hilo (resultado distinto): la suma en paralelo
de gradientes (`#pragma omp atomic`) no es asociativa en punto flotante, y esa
variación mínima se amplifica a través de las iteraciones no lineales de Nesterov.
Se documentó como característica conocida en vez de sacrificar el paralelismo ya
reportado como optimización.

## 3. Bug de rendimiento en la legalización

Al investigar por qué `newblue1` tardaba 15-20 minutos solo en legalizar, se
encontró que `spill_overflow()` ordenaba las celdas de una fila sobrecargada con
insertion sort (O(n²)); con overflow alto una fila puede juntar miles de celdas.
Se cambió a `qsort` (O(n log n)), sin tocar la lógica del algoritmo — se verificó
que los tres benchmarks que ya funcionaban bien dieron el mismo resultado antes y
después del cambio.

## 4. Documentación (README)

Se usó la IA como apoyo de redacción sobre un borrador del README: sugerencias de
fraseo, orden de las secciones y un tono más formal (tomando como referencia el
estilo de un README de otro curso que el profesor había valorado bien). El
contenido final y la versión que se entrega la redacta el estudiante por su cuenta;
lo generado en esta sesión fue un punto de partida, no el texto definitivo.

## 5. Perfilado en dos computadoras

- Se corrió el script de perfilado (`perf stat`, 5 repeticiones por benchmark) en
  la computadora del estudiante, después de que él mismo ajustara los permisos de
  `perf` en su sistema (la IA no ejecuta comandos con `sudo`).
- Al comparar los resultados propios contra los de un compañero de equipo, se
  encontró que a los datos del estudiante les faltaba la columna de tiempo. Se
  aisló la causa: el sistema usa configuración regional en español
  (`LC_NUMERIC=es_ES.UTF-8`), y `perf stat -x,` escribe los decimales con coma
  — que choca con la coma como separador de columnas del propio CSV, corrompiendo
  la fila de `task-clock`. Se repararon los datos ya recolectados y se corrigió el
  script (`LC_NUMERIC=C`) para que no le pase a nadie más.
- Se generó una tabla comparativa por benchmark entre las dos computadoras
  (Intel vs. AMD), notando una diferencia grande en la métrica de fallos de caché
  que probablemente se debe a que el evento genérico de `perf` mapea a un nivel de
  caché distinto según el fabricante — señalada como observación, no como
  conclusión cerrada.

## 6. Perfilado por etapa (instrumentación con `clock_gettime`)

El enunciado pedía más de 100 muestras por etapa en al menos dos computadoras y un
sistema empotrado; el estudiante decidió que la placa empotrada y las 100+ muestras
no eran viables en este contexto (los benchmarks reales tardan minutos cada uno) y
pidió en su lugar una instrumentación por etapa con una sola corrida por benchmark.
Se agregó medición de tiempo (`clock_gettime`, tiempo de pared) alrededor de cada
etapa del pipeline — parseo, initial placement, ciclo de Nesterov, legalización,
escritura de salida — sin romper la firma pública usada por otros módulos. Hallazgo
principal: el ciclo de Nesterov domina el tiempo total (>95% en los cuatro
benchmarks reales), el resto de las etapas es marginal.

## 7. Flujo de git

La IA propuso los comandos de git (crear rama, resolver conflictos de un PR,
fusionar con `develop`) pero **no ejecutó ningún commit ni push sin que el
estudiante lo pidiera explícitamente en cada caso** — varias veces el estudiante
corrió los comandos él mismo y le pidió a la IA solo verificar el resultado.
