#include "fb.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdint>

static const uint64_t SEMILLA = 1938;

static const std::string A1 = "abcdefghijklmnopqrstuvwxyz";              
static const std::string A2 = "abcdefghijklmnopqrstuvwxyz0123456789";      

static const int A1_MIN_LEN = 3, A1_MAX_LEN = 6;
static const int A2_MIN_LEN = 3, A2_MAX_LEN = 5;

static const std::string REF_PASSWORD_INFO = "abc12";
static const std::string REF_HASH =
    fb::sha256Hex("abc12");

struct Instancia {
    std::string nombre;      
    std::string alphabet;    
    int minLength;
    int maxLength;
    std::string targetHash;
    std::string passwordReal; 
};

static std::vector<std::string> generarContrasenasEquipo(uint64_t semilla) {
    const uint64_t MULT = 1103515245ULL;
    const uint64_t INC  = 12345ULL;
    const uint64_t MOD  = 2147483648ULL; 

    std::vector<std::string> alfabetosPorInstancia = {A1, A2, A1, A2, A1};
    std::vector<int> longitudes = {4, 4, 5, 5, 6};

    std::vector<std::string> resultado;
    uint64_t x = semilla; 

    for (size_t inst = 0; inst < longitudes.size(); inst++) {
        std::string pwd;
        const std::string& alpha = alfabetosPorInstancia[inst];

        for (int pos = 0; pos < longitudes[inst]; pos++) {
            int idx = (int)(x % alpha.size());
            pwd += alpha[idx];
            x = (MULT * x + INC) % MOD; 
        }
        resultado.push_back(pwd);
    }

    return resultado;
}

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

void runFB() {
    std::cout << "=== Modulo FB - Fuerza Bruta ===\n";
    std::cout << "Semilla del equipo: " << SEMILLA << "\n\n";

    std::vector<Instancia> instancias;

    instancias.push_back(Instancia{
        "Referencia", A2, A2_MIN_LEN, A2_MAX_LEN, REF_HASH, REF_PASSWORD_INFO
    });

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

    std::cout << "Contraseñas objetivo generadas (sinteticas, propias del equipo):\n";
    for (size_t i = 0; i < instancias.size(); i++) {
        std::cout << "  " << instancias[i].nombre << ": "
                   << instancias[i].passwordReal
                   << "  | hash: " << instancias[i].targetHash << "\n";
    }
    std::cout << "\n";

    const std::string RUTA_DICCIONARIO = "resources/diccionario.txt";

    imprimirEncabezado();

    long long totalCandidatosFB = 0;
    long long totalCandidatosDicc = 0;
    int encontradasFB = 0;
    int encontradasDicc = 0;

    for (const Instancia& inst : instancias) {
        fb::FBConfig config{inst.alphabet, inst.minLength, inst.maxLength, inst.targetHash};
        fb::FBResult resultadoFB = fb::bruteForceSearch(config);
        imprimirFila(inst.nombre, "FB", resultadoFB);

        fb::FBResult resultadoDicc = fb::dictionaryAttack(inst.targetHash, RUTA_DICCIONARIO);
        imprimirFila(inst.nombre, "Dicc", resultadoDicc);

        totalCandidatosFB += resultadoFB.candidatesEvaluated;
        totalCandidatosDicc += resultadoDicc.candidatesEvaluated;
        if (resultadoFB.found) encontradasFB++;
        if (resultadoDicc.found) encontradasDicc++;
    }

    std::cout << "\n=== Resumen comparativo FB vs. Diccionario ===\n";
    std::cout << "Total de instancias evaluadas: " << instancias.size() << "\n";
    std::cout << "Fuerza bruta  -> encontradas: " << encontradasFB
               << "/" << instancias.size()
               << " | candidatos evaluados (suma): " << totalCandidatosFB << "\n";
    std::cout << "Diccionario   -> encontradas: " << encontradasDicc
               << "/" << instancias.size()
               << " | candidatos evaluados (suma): " << totalCandidatosDicc << "\n";
}
