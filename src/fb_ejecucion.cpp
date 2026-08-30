#include "fb_fuerzabruta.hpp"
#include "semilla.hpp"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <cstdint>
#include <sstream>
#include <fstream>
#include <algorithm>


static const std::vector<std::string> APELLIDOS = {"Martinez", "Molina", "Diaz"};
static const uint64_t SEMILLA = semilla::calcularSemilla(APELLIDOS);

static const std::string A1 = "abcdefghijklmnopqrstuvwxyz";
static const std::string A2 = "abcdefghijklmnopqrstuvwxyz0123456789";

static const int A1_MIN_LEN = 3, A1_MAX_LEN = 6;
static const int A2_MIN_LEN = 3, A2_MAX_LEN = 5;

static const std::string REF_PASSWORD_INFO = "abc12";
static const std::string REF_HASH = fb::sha256Hex("abc12");


namespace {

struct Instancia {
    std::string nombre;
    std::string alphabet;
    int minLength;
    int maxLength;
    std::string targetHash;
    std::string passwordReal;
};

void imprimirEncabezado(std::ostream& out) {
    out << std::left
        << std::setw(14) << "Instancia"
        << std::setw(10) << "Metodo"
        << std::setw(12) << "Encontrada"
        << std::setw(16) << "Contraseña"
        << std::setw(18) << "Candidatos"
        << std::setw(12) << "Tiempo(ms)"
        << "\n";
    out << std::string(82, '-') << "\n";
}

static void imprimirFila(std::ostream& out, const std::string& instancia,
                          const std::string& metodo, const fb::FBResult& r) {
    out << std::left
        << std::setw(14) << instancia
        << std::setw(10) << metodo
        << std::setw(12) << (r.found ? "SI" : "NO")
        << std::setw(16) << (r.found ? r.password : "-")
        << std::setw(18) << r.candidatesEvaluated
        << std::setw(12) << std::fixed << std::setprecision(3) << r.elapsedMs
        << "\n";
}

static void escribirCSVFila(std::ofstream& csv, const std::string& instancia,
                             const std::string& metodo, const fb::FBResult& r) {
    csv << "\"" << instancia << "\"," << metodo << "," << (r.found ? "SI" : "NO") << ","
        << "\"" << (r.found ? r.password : "") << "\"," << r.candidatesEvaluated << ","
        << std::fixed << std::setprecision(3) << r.elapsedMs << "\n";
}

}  // namespace (fin del namespace anonimo)

void runFB() {
    // Acumula todo lo impreso para volcarlo tambien a results/results_fb.txt
    std::ostringstream out;

    out << "=== Modulo FB - Fuerza Bruta ===\n";
    out << "Apellidos (orden alfabetico, sin tildes): ";
    {
        std::vector<std::string> normalizados;
        for (const auto& a : APELLIDOS) normalizados.push_back(semilla::normalizarApellido(a));
        std::sort(normalizados.begin(), normalizados.end());
        for (size_t i = 0; i < normalizados.size(); i++) {
            if (i) out << ", ";
            out << normalizados[i];
        }
    }
    out << "\n";
    out << "Semilla del equipo: " << SEMILLA << "\n\n";

    std::vector<Instancia> instancias;

    instancias.push_back(Instancia{
        "Referencia", A2, A2_MIN_LEN, A2_MAX_LEN, REF_HASH, REF_PASSWORD_INFO
    });

    std::vector<std::string> alfabetosPorInstancia = {A1, A2, A1, A2, A1};
    std::vector<int> longitudes = {4, 4, 5, 5, 6};
    std::vector<std::string> contrasenasEquipo =
        semilla::generarContrasenasEquipo(SEMILLA, alfabetosPorInstancia, longitudes);

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

    out << "Contraseñas objetivo generadas (sinteticas, propias del equipo):\n";
    for (size_t i = 0; i < instancias.size(); i++) {
        out << "  " << instancias[i].nombre << ": "
            << instancias[i].passwordReal
            << "  | hash: " << instancias[i].targetHash << "\n";
    }
    out << "\n";

    const std::string RUTA_DICCIONARIO = "resources/diccionario.txt";

    imprimirEncabezado(out);

    std::ofstream csv("results/results_fb.csv");
    csv << "instancia,metodo,encontrada,contrasena,candidatos_evaluados,tiempo_ms\n";

    long long totalCandidatosFB = 0;
    long long totalCandidatosDicc = 0;
    int encontradasFB = 0;
    int encontradasDicc = 0;

    for (const Instancia& inst : instancias) {
        fb::FBConfig config{inst.alphabet, inst.minLength, inst.maxLength, inst.targetHash};
        fb::FBResult resultadoFB = fb::bruteForceSearch(config);
        imprimirFila(out, inst.nombre, "FB", resultadoFB);
        escribirCSVFila(csv, inst.nombre, "FB", resultadoFB);

        fb::FBResult resultadoDicc = fb::dictionaryAttack(inst.targetHash, RUTA_DICCIONARIO);
        imprimirFila(out, inst.nombre, "Dicc", resultadoDicc);
        escribirCSVFila(csv, inst.nombre, "Dicc", resultadoDicc);

        totalCandidatosFB += resultadoFB.candidatesEvaluated;
        totalCandidatosDicc += resultadoDicc.candidatesEvaluated;
        if (resultadoFB.found) encontradasFB++;
        if (resultadoDicc.found) encontradasDicc++;
    }
    csv.close();

    out << "\n=== Resumen comparativo FB vs. Diccionario ===\n";
    out << "Total de instancias evaluadas: " << instancias.size() << "\n";
    out << "Fuerza bruta  -> encontradas: " << encontradasFB
        << "/" << instancias.size()
        << " | candidatos evaluados (suma): " << totalCandidatosFB << "\n";
    out << "Diccionario   -> encontradas: " << encontradasDicc
        << "/" << instancias.size()
        << " | candidatos evaluados (suma): " << totalCandidatosDicc << "\n";
    out << "Resultados guardados en results/results_fb.csv y results/results_fb.txt\n";

    std::cout << out.str();
    std::ofstream txt("results/results_fb.txt");
    txt << out.str();
}
