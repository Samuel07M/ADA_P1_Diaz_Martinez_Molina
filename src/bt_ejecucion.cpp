#include "bt_backtracking.hpp"
#include "semilla.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <algorithm>

// Mismos apellidos y mismo procedimiento de calculo de semilla que
// fb_ejecucion.cpp (ver src/semilla.hpp) -- ambos modulos comparten una
// unica fuente de verdad para la semilla del equipo, en vez de cada uno
// hardcodear su propio valor por separado.
static const std::vector<std::string> APELLIDOS = {"Martinez", "Molina", "Diaz"};
static const uint64_t SEMILLA = semilla::calcularSemilla(APELLIDOS);

// Namespace anonimo: Instancia es un nombre de tipo global (no "static",
// porque los tipos no pueden serlo) que fb_ejecucion.cpp tambien declara
// con un layout distinto (otros campos). Al enlazarse ambos archivos en
// un unico ejecutable (src/main.cpp), sin namespace anonimo el compilador
// fusiona el destructor implicito de Instancia entre las dos definiciones
// (mismo nombre mangled, distinto layout) -- violacion de la regla de
// una sola definicion (ODR) que corrompe memoria en tiempo de ejecucion
// (se detecto con un segfault dentro de ~Instancia() al correr el menu
// con la opcion 2, y se confirmo con AddressSanitizer). El namespace
// anonimo le da enlace interno al tipo y elimina la colision.
namespace {

struct Instancia {
    std::string nombre;
    int n;
    Policy policy;
};

struct Resultado {
    Instancia inst;
    SearchStats conPoda;
    SearchStats sinPoda;
    double horasEstimadasPeorCaso = -1.0;
};

static Resultado correrInstancia(const std::vector<char>& alphabet, const Instancia& inst, long long maxNodes) {
    Resultado r;
    r.inst = inst;
    r.conPoda = backtrackConPoda(alphabet, inst.n, inst.policy, nullptr, 0, maxNodes);
    r.sinPoda = fuerzaBrutaSinPoda(alphabet, inst.n, inst.policy);

    if (r.conPoda.nodeLimitReached && !r.sinPoda.exhaustiveSkipped) {
        double tasaNodosPorSeg = r.conPoda.nodesVisited / (r.conPoda.timeMs / 1000.0);
        r.horasEstimadasPeorCaso = (double)r.sinPoda.nodesGenerated / tasaNodosPorSeg / 3600.0;
    } else if (r.conPoda.nodeLimitReached && r.sinPoda.exhaustiveSkipped && r.sinPoda.nodesGenerated > 0) {
        double tasaNodosPorSeg = r.conPoda.nodesVisited / (r.conPoda.timeMs / 1000.0);
        r.horasEstimadasPeorCaso = (double)r.sinPoda.nodesGenerated / tasaNodosPorSeg / 3600.0;
    }
    return r;
}

static void imprimir(std::ostream& out, const Resultado& r) {
    const auto& inst = r.inst;
    out << std::left << std::setw(34) << inst.nombre
        << " n=" << inst.n
        << "  minL=" << inst.policy.minLower << " minU=" << inst.policy.minUpper
        << " minD=" << inst.policy.minDigit << " minS=" << inst.policy.minSymbol << "\n";

    if (r.conPoda.nodeLimitReached) {
        out << "  [NO TERMINO] Nodos visitados hasta el limite: " << r.conPoda.nodesVisited
            << " en " << std::fixed << std::setprecision(3) << r.conPoda.timeMs << " ms\n";
        out << "  Soluciones halladas (parcial, subestimado): " << r.conPoda.solutionsFound << "\n";
        if (r.horasEstimadasPeorCaso >= 0)
            out << "  Estimado peor caso para completar: " << std::fixed << std::setprecision(2)
                << r.horasEstimadasPeorCaso << " horas\n";
    } else {
        out << "  Nodos visitados (con poda, COMPLETO): " << r.conPoda.nodesVisited << "\n";
        out << "  Soluciones encontradas: " << r.conPoda.solutionsFound << "\n";
        out << "  Tiempo con poda: " << std::fixed << std::setprecision(3) << r.conPoda.timeMs << " ms\n";
        if (!r.sinPoda.exhaustiveSkipped) {
            double reduccion = 100.0 * (1.0 - (double)r.conPoda.nodesVisited / (double)r.sinPoda.nodesGenerated);
            out << "  Nodos sin poda (COMPLETO): " << r.sinPoda.nodesGenerated << "\n";
            out << "  Reduccion del espacio: " << std::fixed << std::setprecision(2) << reduccion << "%\n";
            out << "  Tiempo sin poda: " << std::fixed << std::setprecision(3) << r.sinPoda.timeMs << " ms\n";
            if (r.conPoda.solutionsFound != r.sinPoda.solutionsFound)
                out << "  *** ADVERTENCIA: soluciones no coinciden ***\n";
        } else {
            out << "  Nodos sin poda (teorico, no ejecutado): " << r.sinPoda.nodesGenerated << "\n";
        }
    }
    out << "\n";
}

static void escribirCSV(std::ofstream& csv, const Resultado& r) {
    const auto& inst = r.inst;
    csv << "\"" << inst.nombre << "\"," << inst.n << ","
        << inst.policy.minLower << "," << inst.policy.minUpper << ","
        << inst.policy.minDigit << "," << inst.policy.minSymbol << ","
        << (r.conPoda.nodeLimitReached ? "parcial" : "completo") << ","
        << r.conPoda.nodesVisited << ","
        << r.sinPoda.nodesGenerated << ","
        << r.conPoda.solutionsFound << ","
        << r.conPoda.timeMs << ","
        << (r.sinPoda.exhaustiveSkipped ? -1.0 : r.sinPoda.timeMs) << ","
        << r.horasEstimadasPeorCaso << "\n";
}

}  // namespace (fin del namespace anonimo)

void runBT(long long maxNodes) {
    std::ostringstream out;  // acumula todo lo impreso para volcarlo tambien a results/results_bt.txt

    std::vector<char> alphabet = buildAlphabet();
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
    out << "Semilla del equipo: " << SEMILLA << "\n";
    out << "Tamano del alfabeto: " << alphabet.size() << " simbolos\n";
    out << "Limite de nodos por corrida: " << maxNodes << "\n\n";

    // Parametros de la politica del equipo derivados de la semilla
    // (Seccion 9.2): minLower = 2 + (semilla mod 3); minUpper = 1 +
    // (semilla mod 2); minDigit = 1 + (semilla mod 3); minSymbol = 1.
    semilla::ParametrosPoliticaBT params = semilla::derivarPoliticaBT(SEMILLA);
    const int N_POLITICA = 8;
    int suma = params.minLower + params.minUpper + params.minDigit + params.minSymbol;
    out << "Politica del equipo derivada de la semilla: minLower=" << params.minLower
        << " minUpper=" << params.minUpper << " minDigit=" << params.minDigit
        << " minSymbol=" << params.minSymbol << " (suma=" << suma << ", n=" << N_POLITICA << ")\n";
    if (suma > N_POLITICA) {
        // No ocurre con la semilla actual (suma maxima posible = 3+2+3+1=9
        // solo si los tres modulos dieran su valor mas alto simultaneamente;
        // se deja la salvaguarda explicita que exige el enunciado).
        int exceso = suma - N_POLITICA;
        params.minLower = std::max(0, params.minLower - exceso);
        suma = params.minLower + params.minUpper + params.minDigit + params.minSymbol;
        out << "  *** minLower + minUpper + minDigit + minSymbol > n=8: se redujo minLower en "
            << exceso << " (reportado en el informe). Nueva suma=" << suma << " ***\n";
    }
    out << "\n";

    Policy politicaCompleta{params.minLower, params.minUpper, params.minDigit, params.minSymbol, true};
    Policy politicaRelajada{1, 0, 0, 0, true};
    Policy sinRestricciones{0, 0, 0, 0, true};

    std::vector<Instancia> requeridas = {
        {"Referencia",                        6,  Policy{2,1,1,1,true}},
        {"1. Politica completa",              8,  politicaCompleta},
        {"2. Politica completa n=6",          6,  politicaCompleta},
        {"3. Politica completa n=10",         10, politicaCompleta},
        {"4. Politica relajada",              8,  politicaRelajada},
        {"5. Sin restricciones (poda nula)",  6,  sinRestricciones},
    };

    std::vector<Instancia> barrido;
    for (int n = 4; n <= 7; ++n)
        barrido.push_back({"Barrido n=" + std::to_string(n), n, politicaCompleta});

    std::ofstream csv("results/results_bt.csv");
    csv << "instancia,n,minLower,minUpper,minDigit,minSymbol,estado,"
           "nodos_visitados_con_poda,nodos_sin_poda,soluciones,"
           "tiempo_ms_con_poda,tiempo_ms_sin_poda,horas_estimadas_peor_caso\n";

    out << "=== BARRIDO SUPLEMENTARIO ===\n\n";
    for (const auto& inst : barrido) {
        Resultado r = correrInstancia(alphabet, inst, maxNodes);
        imprimir(out, r);
        escribirCSV(csv, r);
    }

    out << "=== INSTANCIAS REQUERIDAS ===\n\n";
    for (const auto& inst : requeridas) {
        Resultado r = correrInstancia(alphabet, inst, maxNodes);
        imprimir(out, r);
        escribirCSV(csv, r);
    }

    csv.close();
    out << "Resultados guardados en results/results_bt.csv y results/results_bt.txt\n";

    std::cout << out.str();
    std::ofstream txt("results/results_bt.txt");
    txt << out.str();
}
