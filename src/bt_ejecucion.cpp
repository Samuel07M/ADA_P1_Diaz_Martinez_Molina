#include "bt_backtracking.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>

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

static void imprimir(const Resultado& r) {
    const auto& inst = r.inst;
    std::cout << inst.nombre 
              << " | n=" << inst.n
              << " minL=" << inst.policy.minLower << " minU=" << inst.policy.minUpper
              << " minD=" << inst.policy.minDigit << " minS=" << inst.policy.minSymbol << "\n";

    if (r.conPoda.nodeLimitReached) {
        std::cout << "  [NO TERMINO] Nodos visitados hasta el limite: " << r.conPoda.nodesVisited
                  << " en " << std::fixed << std::setprecision(3) << r.conPoda.timeMs << " ms\n";
        std::cout << "  Soluciones halladas (estimado): " << r.conPoda.solutionsFound << "\n";
        if (r.horasEstimadasPeorCaso >= 0)
            std::cout << "  Estimado peor caso para completar: " << std::fixed << std::setprecision(2)
                      << r.horasEstimadasPeorCaso << " horas\n";
    } else {
        std::cout << "  Nodos visitados (con poda): " << r.conPoda.nodesVisited << "\n";
        std::cout << "  Soluciones encontradas: " << r.conPoda.solutionsFound << "\n";
        std::cout << "  Tiempo con poda: " << std::fixed << std::setprecision(3) << r.conPoda.timeMs << " ms\n";
        if (!r.sinPoda.exhaustiveSkipped) {
            double reduccion = 100.0 * (1.0 - (double)r.conPoda.nodesVisited / (double)r.sinPoda.nodesGenerated);
            std::cout << "  Nodos sin poda (completo): " << r.sinPoda.nodesGenerated << "\n";
            std::cout << "  Reduccion del espacio: " << std::fixed << std::setprecision(2) << reduccion << "%\n";
            std::cout << "  Tiempo sin poda: " << std::fixed << std::setprecision(3) << r.sinPoda.timeMs << " ms\n";
            if (r.conPoda.solutionsFound != r.sinPoda.solutionsFound)
                std::cout << "ADVERTENCIA: soluciones no coinciden\n";
        } else {
            std::cout << "  Nodos sin poda (teorico): " << r.sinPoda.nodesGenerated << "\n";
        }
    }
    std::cout << "\n";
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

void runBT(long long maxNodes) {
    std::vector<char> alphabet = buildAlphabet();
    std::cout << "Tamano del alfabeto: " << alphabet.size() << " simbolos\n";
    std::cout << "Limite de nodos por corrida: " << maxNodes << "\n\n";

    Policy politicaCompleta{2, 1, 1, 1, true};
    Policy politicaRelajada{1, 0, 0, 0, true};
    Policy sinRestricciones{0, 0, 0, 0, true};

    std::vector<Instancia> requeridas = {
        {"Referencia", 6, Policy{2,1,1,1,true}},
        {"1. Politica completa", 8, politicaCompleta},
        {"2. Politica completa n=6", 6, politicaCompleta},
        {"3. Politica completa n=10", 10, politicaCompleta},
        {"4. Politica relajada", 8, politicaRelajada},
        {"5. Sin restricciones (poda nula)", 6, sinRestricciones},
    };

    std::vector<Instancia> barrido;
    for (int n = 4; n <= 7; ++n)
        barrido.push_back({"Barrido n=" + std::to_string(n), n, politicaCompleta});

    std::ofstream csv("results_bt.csv");
    csv << "instancia,n,minLower,minUpper,minDigit,minSymbol,estado,"
           "nodos_visitados_con_poda,nodos_sin_poda,soluciones,"
           "tiempo_ms_con_poda,tiempo_ms_sin_poda,horas_estimadas_peor_caso\n";

    std::cout << "=== BARRIDO SUPLEMENTARIO ===\n\n";
    for (const auto& inst : barrido) {
        Resultado r = correrInstancia(alphabet, inst, maxNodes);
        imprimir(r);
        escribirCSV(csv, r);
    }

    std::cout << "=== INSTANCIAS REQUERIDAS ===\n\n";
    for (const auto& inst : requeridas) {
        Resultado r = correrInstancia(alphabet, inst, maxNodes);
        imprimir(r);
        escribirCSV(csv, r);
    }

    csv.close();
    std::cout << "Resultados guardados en results_bt.csv\n";
}
