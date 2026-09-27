# Aceleración heterogénea de un ubicador analítico de P&R sobre plataformas embebidas


Fiorela Chavarría · Fabián Parreaguirre · Brayan Rodríguez

## Actualización

Ya están hechos los primeros 3 issues.

- **#1 – Estructuras y firmas**: las structs del circuito (celdas, redes, pines) y los
  `.h` de todos los módulos. Se armó parecido a **RePlAce**: celda con posición y con
  bandera de "fija", pin guardado como un desplazamiento respecto al centro de su
  celda. Es un modelo pensado para ASIC (como el nuestro), sin los conceptos de FPGA
  (sites, recursos, clock regions) que sí maneja OpenPARF.
- **#2 – Parser de bookshelf**: lee los archivos del circuito (`.aux`, `.nodes`,
  `.nets`, `.pl`) y arma el circuito en memoria. Aquí la referencia es **OpenPARF**:
  es el único de los dos que lee bookshelf (RePlAce solo lo puede *escribir*, no
  tiene lector). Eso sí, ellos lo hacen con un generador de parsers (flex/bison) y
  nosotros lo hicimos a mano, línea por línea.
- **#3 – Wirelength**: calcula el HPWL (el número con el que se mide qué tan bueno es
  un ubicador) y una versión suavizada con gradiente, para mover las piezas más
  adelante. Sigue el mismo patrón de **OpenPARF**: el HPWL exacto va aparte de la
  versión suave, y esta última usa el mismo esquema con `gamma` red por red.

Ya lo probamos no solo con el circuito de juguete: en `bench/ispd2005` corrimos
`adaptec1`, un circuito real del concurso ISPD 2005 con cerca de 211 mil celdas.

## Qué hay en cada carpeta

- **`include/`** — los `.h` de cada módulo (`netlist`, `bookshelf`, `wirelength`,
  `density`, `optimizer`, `legalize`, `grid`, más dos ayudas internas `lineio` y
  `strmap`). Solo anuncian qué existe, sin código adentro.
- **`src/`** — la implementación real, un archivo por responsabilidad: el lector de
  bookshelf, las estructuras del circuito, el wirelength, las dos ayudas internas y
  `main.c`.
- **`tests/`** — `test_bookshelf.c` y `test_wirelength.c`, se corren con `make test`.
- **`bench/`** — los circuitos de prueba: `toy/` (uno chiquito inventado por nosotros,
  resultado calculado a mano) e `ispd2005/` (circuitos reales, como `adaptec1`). No se
  sube a git por el tamaño; cada quien lo descarga aparte.
- **`build/`** — lo que genera `make` al compilar. No se sube a git y se borra sin
  problema con `make clean`.

## Cómo probarlo

Con el benchmark real (hay que descargarlo antes, no viene en el repo):

```bash
cd Proyecto
./build/placer bench/ispd2005/adaptec1.aux
```

```
celdas: 211447  redes: 221142  pines: 944053
HPWL inicial: 1.049242e+08
```

## Qué falta

Densidad (#4), el ciclo que junta todo y mueve las piezas (#5), legalización y salida
(#6), y afinar build/benchmarks (#7). Eso primero en CPU; la aceleración en GPU viene
después, sin agregar funciones nuevas, solo haciendo correr más rápido lo que ya
funciona.
