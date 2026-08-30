
#include "test_utils.hpp"
#include "../src/bt_backtracking.hpp"
#include "../src/semilla.hpp"
#include <set>

static void testClassify() {
    std::cout << "\n-- classify() --\n";
    CHECK(classify('a') == CharType::LOWER, "'a' se clasifica como LOWER");
    CHECK(classify('z') == CharType::LOWER, "'z' se clasifica como LOWER");
    CHECK(classify('A') == CharType::UPPER, "'A' se clasifica como UPPER");
    CHECK(classify('Z') == CharType::UPPER, "'Z' se clasifica como UPPER");
    CHECK(classify('0') == CharType::DIGIT, "'0' se clasifica como DIGIT");
    CHECK(classify('9') == CharType::DIGIT, "'9' se clasifica como DIGIT");
    CHECK(classify('!') == CharType::SYMBOL, "'!' se clasifica como SYMBOL");
    CHECK(classify('%') == CharType::SYMBOL, "'%' se clasifica como SYMBOL");
}

static void testAlfabetoBase() {
    std::cout << "\n-- buildAlphabet() --\n";
    std::vector<char> a = buildAlphabet();
   
    CHECK_EQ(a.size(), (size_t)67,
             "el alfabeto base tiene 67 simbolos (26+26+10+5; el enunciado dice \"69\" pero solo lista 5 simbolos -- ver comentario, consultar al docente)");
    std::set<char> unicos(a.begin(), a.end());
    CHECK_EQ(unicos.size(), a.size(), "el alfabeto base no tiene simbolos repetidos");
}

static void testEsFactible() {
    std::cout << "\n-- esFactible() (poda por factibilidad) --\n";
    Policy p{2, 1, 1, 1, true};  // n implicito = 8 en los ejemplos de abajo

    Counts vacio{0, 0, 0, 0};
    CHECK(esFactible(vacio, 0, 8, p), "prefijo vacio con n=8 y deficit=5 es factible (5<=8)");

    Counts vacio2{0, 0, 0, 0};
    CHECK(!esFactible(vacio2, 5, 8, p), "prefijo vacio de longitud 5 con solo 3 posiciones restantes NO es factible (deficit=5>3)");

    Counts completo{2, 1, 1, 1};
    CHECK(esFactible(completo, 4, 8, p), "una vez satisfechos los minimos, cualquier longitud restante es factible");
}

static void testEsSolucionValida() {
    std::cout << "\n-- esSolucionValida() --\n";
    Policy p{2, 1, 1, 1, true};
    CHECK(esSolucionValida(Counts{2, 1, 1, 1}, p), "contadores que igualan los minimos son una solucion valida");
    CHECK(esSolucionValida(Counts{3, 2, 2, 2}, p), "contadores que superan los minimos son una solucion valida");
    CHECK(!esSolucionValida(Counts{1, 1, 1, 1}, p), "falta un minLower -> no es una solucion valida");
    CHECK(!esSolucionValida(Counts{2, 0, 1, 1}, p), "falta un minUpper -> no es una solucion valida");
}

static void testDerivarPoliticaBT() {
    std::cout << "\n-- derivarPoliticaBT() (Seccion 9.2) --\n";
    uint64_t semillaEquipo = semilla::calcularSemilla({"Martinez", "Molina", "Diaz"});
    CHECK_EQ(semillaEquipo, (uint64_t)1938, "la semilla del equipo es 1938 (consistente con test_fb.cpp)");

    semilla::ParametrosPoliticaBT params = semilla::derivarPoliticaBT(semillaEquipo);
    // 1938 mod 3 = 0 ; 1938 mod 2 = 0
    CHECK_EQ(params.minLower, 2, "minLower = 2 + (1938 mod 3) = 2");
    CHECK_EQ(params.minUpper, 1, "minUpper = 1 + (1938 mod 2) = 1");
    CHECK_EQ(params.minDigit, 1, "minDigit = 1 + (1938 mod 3) = 1");
    CHECK_EQ(params.minSymbol, 1, "minSymbol = 1 (fijo)");

    int suma = params.minLower + params.minUpper + params.minDigit + params.minSymbol;
    CHECK(suma <= 8, "minLower+minUpper+minDigit+minSymbol <= n=8 (no hace falta reducir minLower)");
}


static void testPodaEquivaleAExhaustivo() {
    std::cout << "\n-- Con poda vs. sin poda: mismas soluciones (correctitud) --\n";
    std::vector<char> alfabetoChico = {'a', 'b', 'A', 'B', '1', '2', '!'};

    struct Caso { std::string nombre; int n; Policy policy; };
    std::vector<Caso> casos = {
        {"politica exigente, n=4", 4, Policy{1, 1, 1, 1, true}},
        {"politica relajada, n=5", 5, Policy{1, 0, 0, 0, true}},
        {"sin restricciones (poda nula), n=3", 3, Policy{0, 0, 0, 0, true}},
        {"politica imposible (minimos exceden n), n=3", 3, Policy{2, 2, 2, 2, true}},
    };

    for (const auto& caso : casos) {
        SearchStats conPoda = backtrackConPoda(alfabetoChico, caso.n, caso.policy);
        SearchStats sinPoda = fuerzaBrutaSinPoda(alfabetoChico, caso.n, caso.policy);

        CHECK(!sinPoda.exhaustiveSkipped, "caso \"" + caso.nombre + "\": el espacio es lo bastante chico para enumerarlo completo");
        CHECK(!conPoda.nodeLimitReached, "caso \"" + caso.nombre + "\": la version con poda termina sin tocar el limite de nodos");
        CHECK_EQ(conPoda.solutionsFound, sinPoda.solutionsFound,
                 "caso \"" + caso.nombre + "\": con poda y sin poda encuentran el mismo numero de soluciones");
        CHECK(conPoda.nodesVisited <= sinPoda.nodesGenerated,
              "caso \"" + caso.nombre + "\": la poda nunca visita mas nodos que la enumeracion exhaustiva");
    }
}

static void testPodaReduceEspacioEnCasoRestrictivo() {
    std::cout << "\n-- La poda reduce efectivamente el espacio explorado --\n";
    std::vector<char> alfabetoChico = {'a', 'b', 'A', 'B', '1', '2', '!'};
    
    Policy restrictiva{2, 2, 2, 1, true};
    int n = 7;

    SearchStats conPoda = backtrackConPoda(alfabetoChico, n, restrictiva);
    SearchStats sinPoda = fuerzaBrutaSinPoda(alfabetoChico, n, restrictiva);

    CHECK(!sinPoda.exhaustiveSkipped, "el espacio de prueba es tratable exhaustivamente");
    CHECK_EQ(conPoda.solutionsFound, sinPoda.solutionsFound, "mismo numero de soluciones con y sin poda");
    CHECK(conPoda.nodesVisited < sinPoda.nodesGenerated,
          "con una politica restrictiva, la poda visita estrictamente menos nodos que la enumeracion completa");
}

static void testProhibicionRepetidosConsecutivos() {
    std::cout << "\n-- Prohibicion de caracteres repetidos consecutivos --\n";
    std::vector<char> alfabetoChico = {'a', 'b'};
    Policy sinMinimos{0, 0, 0, 0, true};  // solo aplica la prohibicion de repetidos

    std::vector<std::string> soluciones;
    SearchStats stats = backtrackConPoda(alfabetoChico, 3, sinMinimos, &soluciones, 100);

    
    CHECK_EQ(stats.solutionsFound, (long long)2, "con {a,b} y n=3, solo hay 2 cadenas sin repetidos consecutivos");
    std::set<std::string> encontradas(soluciones.begin(), soluciones.end());
    CHECK(encontradas.count("aba") == 1, "\"aba\" es una de las soluciones");
    CHECK(encontradas.count("bab") == 1, "\"bab\" es una de las soluciones");
}

int main() {
    testClassify();
    testAlfabetoBase();
    testEsFactible();
    testEsSolucionValida();
    testDerivarPoliticaBT();
    testPodaEquivaleAExhaustivo();
    testPodaReduceEspacioEnCasoRestrictivo();
    testProhibicionRepetidosConsecutivos();
    return resumenFinal("tests/test_bt.cpp");
}
