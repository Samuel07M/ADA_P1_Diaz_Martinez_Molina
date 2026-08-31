# ADA_P1_Diaz_Martinez_Molina

Práctica 1 — Fuerza Bruta y Backtracking · Análisis y Diseño de Algoritmos

Contraseñas bajo ataque y bajo diseño: enumeración exhaustiva (Módulo FB) y
construcción con poda (Módulo BT) sobre un mismo dominio de contraseñas.

## Integrantes

Ordenados alfabéticamente por apellido (los mismos apellidos usados para
calcular la semilla del equipo):

| Apellido  | Nombre completo          |
|-----------|--------------------------|
| Díaz      | Santiago Diaz Quintero   |
| Martínez  | Juan Diego Martínez      |
| Molina    | Samuel Molina Garcés     |

## Organización del proyecto

El repositorio tiene dos módulos que comparten un mismo dominio
(contraseñas sobre un alfabeto) pero abordan el problema desde roles
opuestos, y un único ejecutable (`src/main.cpp`) que expone ambos por un
menú interactivo. El **Módulo FB** (`src/fb_fuerzabruta.hpp/.cpp`,
`src/fb_ejecucion.cpp`) actúa como atacante que no conoce la contraseña y
la busca probando exhaustivamente todo el espacio de candidatos (fuerza
bruta pura) y, por separado, con un ataque por diccionario sobre
`resources/diccionario.txt`. El **Módulo BT** (`src/bt_backtracking.hpp/.cpp`,
`src/bt_ejecucion.cpp`) actúa como diseñador de una política de
contraseñas que construye candidatos incrementalmente y poda ramas
infactibles, comparando siempre contra una versión sin poda (enumeración
exhaustiva con filtrado) para verificar que ambas encuentran el mismo
número de soluciones. Ambos módulos comparten `src/semilla.hpp/.cpp`,
que centraliza el cálculo de la semilla del equipo y la generación
determinista de instancias (evita reimplementar la misma lógica dos veces y
es lo que permite testear esa lógica una sola vez en `tests/`). La biblioteca
de hashing SHA-256 vive en `src/third_party/picosha2.h` (de terceros, según
lo autoriza la Sección 10 del enunciado). `tests/` contiene la suite de
pruebas automatizadas y la utilidad de verificación de semilla;
`resources/` los datos de entrada; `results/` las salidas generadas
(CSV, TXT, PNG) por cada corrida; y `report/` el informe técnico.

```
ADA_P1_Diaz_Martinez_Molina/
├── README.md
├── src/
│   ├── semilla.hpp / semilla.cpp        # calculo de semilla + generacion de instancias (compartido)
│   ├── fb_fuerzabruta.hpp / .cpp        # Modulo FB: fuerza bruta + ataque por diccionario
│   ├── bt_backtracking.hpp / .cpp       # Modulo BT: backtracking con poda + version sin poda
│   ├── fb_ejecucion.cpp                 # runFB(): corre el Modulo FB (sin main propio)
│   ├── bt_ejecucion.cpp                 # runBT(maxNodes): corre el Modulo BT (sin main propio)
│   ├── main.cpp                         # unico main(): menu interactivo que llama runFB()/runBT()
│   └── third_party/picosha2.h           # SHA-256 (biblioteca de terceros)
├── tests/
│   ├── test_utils.hpp                   # micro-arnes de pruebas (sin dependencias externas)
│   ├── test_fb.cpp                      # pruebas del Modulo FB
│   ├── test_bt.cpp                      # pruebas del Modulo BT
│   └── verificar_semilla.cpp            # utilidad de verificacion de semilla
├── resources/
│   └── diccionario.txt                  # diccionario sintetico (500 candidatos)
├── results/
│   ├── results_fb.csv / results_fb.txt  # salidas del Modulo FB
│   ├── results_bt.csv / results_bt.txt  # salidas del Modulo BT
│   ├── grafica_*.png                    # graficas generadas a partir de los CSV
│   └── graficar.py                      # script que genera las graficas
└── report/
    └── Informe.pdf                      # informe tecnico 
```

## Requisitos

- `g++` con soporte para C++17 (probado con GCC; estándar del curso: `g++ -std=c++17 -O2`).
- Python 3 con `matplotlib` **solo** si se quieren regenerar las gráficas de `results/` (no hace falta para compilar ni correr los módulos en C++).

## Compilación

Desde la raíz del repositorio, un único comando compila todo (Módulo FB +
Módulo BT + el menú) en un solo ejecutable, como exige la Sección 11:

```bash
mkdir -p bin
g++ -std=c++17 -O2 -o bin/ada_p1 src/main.cpp src/fb_ejecucion.cpp src/fb_fuerzabruta.cpp src/bt_ejecucion.cpp src/bt_backtracking.cpp src/semilla.cpp
```

## Ejecución

El ejecutable asume que se corre **desde la raíz del repositorio** (usa
rutas relativas `resources/diccionario.txt` y `results/...`):

```bash
./bin/ada_p1
```

Se muestra este menú interactivo:

```bash
# === MENU DE EJECUCION ===
# 0. Salir
# 1. Fuerza Bruta
# 2. Backtracking
# Opcion:
```

Hay que correrlo dos veces (opción 1 y luego opción 2, en cualquier orden,
antes de salir con 0) para generar los resultados de ambos módulos. Para
generarlos sin interacción manual (por ejemplo, para automatizar la
regeneración de `results/`), se le puede dar la secuencia de opciones por
la entrada estándar:

```bash
printf "1\n2\n0\n" | ./bin/ada_p1
```

La opción **1 (Fuerza Bruta)** reproduce la instancia de referencia común
al curso y las 5 instancias privadas del equipo: imprime una
tabla en pantalla y además escribe `results/results_fb.csv` y
`results/results_fb.txt`. **Advertencia de tiempo:** la instancia de mayor
longitud (alfabeto A1, n=6, hasta 26⁶ ≈ 309 millones de candidatos) puede
tardar varios minutos en una computadora personal — es intencional: es
justo el punto donde el enunciado dice que "el espacio deja de ser
tratable", y es la evidencia empírica que pide la Sección 8
("identificación explícita del punto a partir del cual el crecimiento del
costo deja de ser manejable").

La opción **2 (Backtracking)** reproduce el barrido suplementario (n=4..7)
y las 6 instancias requeridas de la Sección 9.2, con un límite de nodos
por corrida (para que una política casi sin restricciones no corra
indefinidamente); cuando una instancia llega al límite se reporta
explícitamente como `[NO TERMINO]` con una estimación del tiempo que
tomaría completarla, en vez de fingir un resultado completo. Escribe
`results/results_bt.csv` y `results/results_bt.txt`. Usa siempre el
límite de nodos por defecto (50.000.000); ese límite está fijado en
`src/main.cpp` (no se pide por consola, a diferencia de versiones
anteriores de este ejecutable).

Para regenerar las gráficas a partir de los CSV ya generados:

```bash
python3 results/graficar.py
```

## Pruebas automatizadas

Cada suite es su propio ejecutable independiente; se compila y se corre a
mano, uno por uno, desde la raíz del repositorio:

```bash
mkdir -p bin

# utilidad de verificación de semilla
g++ -std=c++17 -O2 -o bin/verificar_semilla tests/verificar_semilla.cpp src/semilla.cpp src/fb_fuerzabruta.cpp
./bin/verificar_semilla

# pruebas del Módulo FB
g++ -std=c++17 -O2 -o bin/test_fb tests/test_fb.cpp src/fb_fuerzabruta.cpp src/semilla.cpp
./bin/test_fb

# pruebas del Módulo BT
g++ -std=c++17 -O2 -o bin/test_bt tests/test_bt.cpp src/bt_backtracking.cpp src/semilla.cpp
./bin/test_bt
```

Cada ejecutable imprime `[OK]` o `[FALLO]` por cada verificación y termina
con un resumen (`X/Y checks OK`); el código de salida es 0 si todo pasó, o
1 si algo falló (`echo $?` justo después de correrlo para confirmarlo). En
total suman 99 verificaciones entre los tres. Ver el encabezado de cada
archivo en `tests/` para el detalle de qué se prueba y por qué.

## La semilla del equipo

Apellidos del equipo, en orden alfabético: **díaz, martínez, molina**.

Procedimiento, reproducible con
`tests/verificar_semilla.cpp`:

1. Normalizar cada apellido: minúsculas, sin espacios, sin tildes.
2. Ordenarlos alfabéticamente: `diaz, martinez, molina`.
3. Concatenarlos sin espacios: `"diazmartinezmolina"` (18 caracteres).
4. Sumar los códigos ASCII de todos los caracteres: **1938**.
5. `semilla = 1938 mod 100000 = 1938`.

La semilla **no está hardcodeada** en ningún `.cpp`: se calcula en tiempo
de ejecución con `semilla::calcularSemilla(...)` (`src/semilla.cpp`), a
partir de los mismos apellidos, en `fb_ejecucion.cpp`, `bt_ejecucion.cpp` y
`tests/verificar_semilla.cpp` — así cualquiera puede confirmar que el
valor no fue elegido a mano.

A partir de la semilla se derivan, de forma determinista:

- Las 5 contraseñas objetivo del Módulo FB (generador congruencial lineal).
- Los parámetros de la política del Módulo BT:
  `minLower=2, minUpper=1, minDigit=1, minSymbol=1` (con `semilla=1938`).

## Notas técnicas y decisiones a reportar

- **Corrección de error en `src/third_party/picosha2.h`:**  
  La implementación de SHA-256 subida originalmente tenía dos errores que 
  hacían que `sha256Hex(x)` devolviera un valor **incorrecto para cualquier
   `x` no vacío**. Aunque el código era autoconsistente (las búsquedas
    "funcionaban" internamente), los hashes no correspondían al estándar
     SHA-256 real. Esto se detectó al comparar el código contra `sha256sum`
      y los vectores de prueba oficiales en `tests/test_fb.cpp`.  
  Los fallos corregidos fueron:
  1. Un dígito hexadecimal mal transcrito en la constante `K` de la tabla
   de rondas (`0xbef4a3f7` en lugar de `0xbef9a3f7`, según el estándar FIPS
    180-4).
  2. El cálculo de la longitud del mensaje en `process()`, el cual usaba 
  `std::distance(first, last)` **después** de que el bucle avanzara `first`
   hasta igualar `last` (dando siempre 0).
- **Unificación de `main_fb.cpp` y `main_bt.cpp` en `src/main.cpp`:**  
  Los ejecutables que antes eran independientes se reestructuraron en 
  funciones sin `main()` propio (`fb_ejecucion.cpp::runFB()` y
   `bt_ejecucion.cpp::runBT(maxNodes)`). Ahora son llamados desde un único
    punto de entrada en `src/main.cpp`, el cual se compila con un solo
     comando y permite seleccionar el módulo mediante un menú interactivo.  
  Durante esta reorganización se identificaron y restauraron tres elementos
   que se habían perdido temporalmente:
  1. `fb_ejecucion.cpp` no estaba guardando los resultados en
   `results/results_fb.csv`/`.txt` (solo imprimía en consola).
  2. La semilla se había vuelto a fijar manualmente al valor literal `1938` e
   incluía una copia local del generador congruencial lineal, en lugar de usar
    `semilla::calcularSemilla(...)` y `semilla::generarContrasenasEquipo(...)`.
  3. Se había vuelto a incluir accidentalmente la versión antigua de `picosha2.h` 
  con los errores de SHA-256 mencionados en el punto anterior.
- **Inconsistencia detectada en el enunciado del proyecto (Sección 9.2):**  
  El enunciado describe el alfabeto base como *"minúsculas, mayúsculas, dígitos 
  y el conjunto de símbolos `{!, @, #, $, %}` (69 símbolos en total)"*. Sin embargo,
  la suma matemática real es 26 + 26 + 10 + 5 = 67, no 69. La función `buildAlphabet()`
  se implementó utilizando estrictamente los 5 símbolos listados de forma explícita
  (67 en total). Para más detalle, consultar el comentario en `tests/test_bt.cpp`.
