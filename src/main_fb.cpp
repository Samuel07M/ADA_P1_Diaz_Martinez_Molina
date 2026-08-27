#include "fb.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdint>

// ===========================================================================
// main.cpp
// Programa que arma las instancias del Modulo FB (segun Seccion 9.1 del
// enunciado), corre fuerza bruta pura y ataque por diccionario sobre cada
// una, y muestra una tabla comparativa en consola.
//
// Este archivo SOLO usa las funciones que ya estan declaradas en fb.hpp:
// sha256Hex, bruteForceSearch y dictionaryAttack. Aqui no se implementa
// ningun algoritmo nuevo, solo se arman los datos de entrada y se
// presentan los resultados.
// ===========================================================================

// ---------------------------------------------------------------------
// Datos fijos del equipo (Diaz, Martinez, Molina)
// ---------------------------------------------------------------------
static const uint64_t SEMILLA = 1938;

// Alfabetos definidos en la Seccion 9.1
static const std::string A1 = "abcdefghijklmnopqrstuvwxyz";                 // 26 simbolos
static const std::string A2 = "abcdefghijklmnopqrstuvwxyz0123456789";       // 36 simbolos

// Rangos de longitud a explorar en fuerza bruta segun el alfabeto (9.1)
static const int A1_MIN_LEN = 3, A1_MAX_LEN = 6;
static const int A2_MIN_LEN = 3, A2_MAX_LEN = 5;

// Instancia de referencia comun a todo el curso (para validar el programa)
static const std::string REF_PASSWORD_INFO = "abc12"; // solo informativo
static const std::string REF_HASH =
    fb::sha256Hex("abc12");

// ---------------------------------------------------------------------
// Estructura simple para describir cada instancia que se va a resolver:
// que alfabeto usar en la busqueda, en que rango de longitudes buscar,
// y el hash objetivo.
// ---------------------------------------------------------------------
struct Instancia {
    std::string nombre;      // etiqueta para la tabla de resultados
    std::string alphabet;    // alfabeto de busqueda (A1 o A2)
    int minLength;
    int maxLength;
    std::string targetHash;
    std::string passwordReal; // solo para mostrarla en consola (dato sintetico propio)
};

// ---------------------------------------------------------------------
// generarContrasenasEquipo
//
// Genera las 5 contrasenas objetivo del equipo usando el generador
// congruencial lineal (LCG) descrito en la Seccion 9.1:
//   x0 = semilla
//   x_{i+1} = (1103515245 * x_i + 12345) mod 2^31
// El caracter i-esimo (indice GLOBAL, continuo a lo largo de las 5
// contrasenas) se obtiene como alfabeto[x_i mod |alfabeto|], y luego
// se avanza x antes de generar el siguiente caracter.
//
// Patron de alfabetos: A1, A2, A1, A2, A1
// Longitudes:           4,  4,  5,  5,  6
// ---------------------------------------------------------------------
static std::vector<std::string> generarContrasenasEquipo(uint64_t semilla) {
    const uint64_t MULT = 1103515245ULL;
    const uint64_t INC  = 12345ULL;
    const uint64_t MOD  = 2147483648ULL; // 2^31

    std::vector<std::string> alfabetosPorInstancia = {A1, A2, A1, A2, A1};
    std::vector<int> longitudes = {4, 4, 5, 5, 6};

    std::vector<std::string> resultado;
    uint64_t x = semilla; // x0

    for (size_t inst = 0; inst < longitudes.size(); inst++) {
        std::string pwd;
        const std::string& alpha = alfabetosPorInstancia[inst];

        for (int pos = 0; pos < longitudes[inst]; pos++) {
            int idx = (int)(x % alpha.size());
            pwd += alpha[idx];
            x = (MULT * x + INC) % MOD; // avanzamos el LCG para el siguiente caracter
        }
        resultado.push_back(pwd);
    }

    return resultado;
}

// ---------------------------------------------------------------------
// imprimirEncabezado / imprimirFila
// Solo formato de tabla en consola, nada algoritmico.
// ---------------------------------------------------------------------
static void imprimirEncabezado() {
    std::cout << std::left
               << std::setw(14) << "Instancia"
               << std::setw(10) << "Metodo"
               << std::setw(12) << "Encontrada"
               << std::setw(16) << "Contraseña"
               << std::setw(18) << "Candidatos"
               << std::setw(12) << "Tiempo(ms)"
               << "\n";
    std::cout << std::string(82, '-') << "\n";
}

static void imprimirFila(const std::string& instancia,
                          const std::string& metodo,
                          const fb::FBResult& r) {
    std::cout << std::left
               << std::setw(14) << instancia
               << std::setw(10) << metodo
               << std::setw(12) << (r.found ? "SI" : "NO")
               << std::setw(16) << (r.found ? r.password : "-")
               << std::setw(18) << r.candidatesEvaluated
               << std::setw(12) << std::fixed << std::setprecision(3) << r.elapsedMs
               << "\n";
}

int main() {
    std::cout << "=== Modulo FB - Fuerza Bruta ===\n";
    std::cout << "Semilla del equipo (Diaz, Martinez, Molina): " << SEMILLA << "\n\n";

    // -----------------------------------------------------------------
    // 1) Armamos la lista completa de instancias a resolver:
    //    - la instancia de referencia del curso
    //    - las 5 instancias propias del equipo
    // -----------------------------------------------------------------
    std::vector<Instancia> instancias;

    // Instancia de referencia (valida que la implementacion sea correcta)
    instancias.push_back(Instancia{
        "Referencia", A2, A2_MIN_LEN, A2_MAX_LEN, REF_HASH, REF_PASSWORD_INFO
    });

    // Instancias propias del equipo
    std::vector<std::string> contrasenasEquipo = generarContrasenasEquipo(SEMILLA);
    std::vector<std::string> alfabetosPorInstancia = {A1, A2, A1, A2, A1};

    for (size_t i = 0; i < contrasenasEquipo.size(); i++) {
        std::string hashObjetivo = fb::sha256Hex(contrasenasEquipo[i]);
        bool esA1 = (alfabetosPorInstancia[i] == A1);

        instancias.push_back(Instancia{
            "Equipo-" + std::to_string(i + 1),
            alfabetosPorInstancia[i],
            esA1 ? A1_MIN_LEN : A2_MIN_LEN,
            esA1 ? A1_MAX_LEN : A2_MAX_LEN,
            hashObjetivo,
            contrasenasEquipo[i]
        });
    }

    // Mostramos las contraseñas generadas (dato sintetico, util para el informe)
    std::cout << "Contraseñas objetivo generadas (sinteticas, propias del equipo):\n";
    for (size_t i = 0; i < instancias.size(); i++) {
        std::cout << "  " << instancias[i].nombre << ": "
                   << instancias[i].passwordReal
                   << "  | hash: " << instancias[i].targetHash << "\n";
    }
    std::cout << "\n";

    // -----------------------------------------------------------------
    // 2) Corremos fuerza bruta pura y ataque por diccionario para
    //    cada instancia, y vamos guardando los resultados.
    // -----------------------------------------------------------------
    const std::string RUTA_DICCIONARIO = "diccionario.txt";

    imprimirEncabezado();

    long long totalCandidatosFB = 0;
    long long totalCandidatosDicc = 0;
    int encontradasFB = 0;
    int encontradasDicc = 0;

    for (const Instancia& inst : instancias) {
        // --- Fuerza bruta pura ---
        fb::FBConfig config{inst.alphabet, inst.minLength, inst.maxLength, inst.targetHash};
        fb::FBResult resultadoFB = fb::bruteForceSearch(config);
        imprimirFila(inst.nombre, "FB", resultadoFB);

        // --- Ataque por diccionario ---
        fb::FBResult resultadoDicc = fb::dictionaryAttack(inst.targetHash, RUTA_DICCIONARIO);
        imprimirFila(inst.nombre, "Dicc", resultadoDicc);

        // Acumulamos metricas para el resumen final
        totalCandidatosFB += resultadoFB.candidatesEvaluated;
        totalCandidatosDicc += resultadoDicc.candidatesEvaluated;
        if (resultadoFB.found) encontradasFB++;
        if (resultadoDicc.found) encontradasDicc++;
    }

    // -----------------------------------------------------------------
    // 3) Resumen final: tasa de exito de cada estrategia. Esto alimenta
    //    directamente la comparacion exigida en la Seccion 8.1.
    // -----------------------------------------------------------------
    std::cout << "\n=== Resumen comparativo FB vs. Diccionario ===\n";
    std::cout << "Total de instancias evaluadas: " << instancias.size() << "\n";
    std::cout << "Fuerza bruta  -> encontradas: " << encontradasFB
               << "/" << instancias.size()
               << " | candidatos evaluados (suma): " << totalCandidatosFB << "\n";
    std::cout << "Diccionario   -> encontradas: " << encontradasDicc
               << "/" << instancias.size()
               << " | candidatos evaluados (suma): " << totalCandidatosDicc << "\n";

    return 0;
}