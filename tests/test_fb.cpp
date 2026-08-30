
#include "test_utils.hpp"
#include "../src/fb_fuerzabruta.hpp"
#include "../src/semilla.hpp"
#include <fstream>
#include <set>

static void testVectoresSHA256(){
    std::cout << "\n-- Vectores de prueba SHA-256 conocidos --\n";
    
    CHECK_EQ(fb::sha256Hex(""),
             std::string("e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"),
             "sha256(vacio)");
    CHECK_EQ(fb::sha256Hex("abc"),
             std::string("ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"),
             "sha256(\"abc\")");
    CHECK_EQ(fb::sha256Hex("abc12"),
             std::string("8d51feb34e3e69f6fa6dffc577e2c60490cf9a7fcd835f9f6af1505b71d74773"),
             "sha256(\"abc12\") == instancia de referencia (Seccion 9.1)");
}

static void testInstanciaReferencia() {
    std::cout << "\n-- Instancia de referencia comun al curso (Seccion 9.1) --\n";
    const std::string A2 = "abcdefghijklmnopqrstuvwxyz0123456789";
    std::string targetHash = fb::sha256Hex("abc12");

    fb::FBConfig config{A2, 3, 5, targetHash};
    fb::FBResult r = fb::bruteForceSearch(config);

    CHECK(r.found, "la busqueda exhaustiva encuentra la instancia de referencia");
    CHECK_EQ(r.password, std::string("abc12"), "la contrasena encontrada es \"abc12\"");
    CHECK(r.candidatesEvaluated > 0, "se evaluo al menos un candidato");
}

static void testAtaqueDiccionario() {
    std::cout << "\n-- Ataque por diccionario --\n";
    const std::string ruta = "resources/diccionario.txt";

    std::ifstream f(ruta);
    CHECK(f.good(), "resources/diccionario.txt existe y se puede abrir (ejecutar desde la raiz del repo)");
    if (!f.good()) return;

    
    std::string hashPassword = fb::sha256Hex("password");
    fb::FBResult r1 = fb::dictionaryAttack(hashPassword, ruta);
    CHECK(r1.found, "dictionaryAttack encuentra \"password\" (esta en diccionario.txt)");
    CHECK_EQ(r1.password, std::string("password"), "dictionaryAttack devuelve la contrasena correcta");

   
    std::string hashInexistente = fb::sha256Hex("esto-no-esta-en-el-diccionario-xyz");
    fb::FBResult r2 = fb::dictionaryAttack(hashInexistente, ruta);
    CHECK(!r2.found, "dictionaryAttack reporta \"no encontrada\" para un hash ausente del diccionario");

   
    std::string hashAbc12 = fb::sha256Hex("abc12");
    fb::FBResult r3 = fb::dictionaryAttack(hashAbc12, ruta);
    CHECK(!r3.found, "\"abc12\" (instancia de referencia) NO esta en el diccionario.txt actual del curso");
}

static void testConsistenciaInstanciasEquipo() {
    std::cout << "\n-- Consistencia de las instancias sinteticas del equipo (Seccion 9.1) --\n";
    const std::string A1 = "abcdefghijklmnopqrstuvwxyz";
    const std::string A2 = "abcdefghijklmnopqrstuvwxyz0123456789";

    uint64_t sem = semilla::calcularSemilla({"Martinez", "Molina", "Diaz"});
    CHECK_EQ(sem, (uint64_t)1938, "la semilla del equipo es 1938");

    std::vector<std::string> alfabetos = {A1, A2, A1, A2, A1};
    std::vector<int> longitudes = {4, 4, 5, 5, 6};
    std::vector<std::string> pwds = semilla::generarContrasenasEquipo(sem, alfabetos, longitudes);

    CHECK_EQ(pwds.size(), (size_t)5, "se generaron 5 contraseñas objetivo");

    
    std::vector<std::string> pwds2 = semilla::generarContrasenasEquipo(sem, alfabetos, longitudes);
    CHECK(pwds == pwds2, "generarContrasenasEquipo es determinista (misma semilla -> mismas contraseñas)");

    for (size_t i = 0; i < pwds.size(); i++) {
        bool esA1 = (alfabetos[i] == A1);
        CHECK_EQ((int)pwds[i].size(), longitudes[i],
                 "instancia " + std::to_string(i + 1) + ": longitud correcta");
        for (char c : pwds[i]) {
            bool enAlfabeto = alfabetos[i].find(c) != std::string::npos;
            CHECK(enAlfabeto, "instancia " + std::to_string(i + 1) +
                                   ": todos los caracteres pertenecen al alfabeto " +
                                   (esA1 ? "A1" : "A2"));
            if (!enAlfabeto) break;
        }
        
        std::string hash1 = fb::sha256Hex(pwds[i]);
        std::string hash2 = fb::sha256Hex(pwds[i]);
        CHECK_EQ(hash1, hash2, "instancia " + std::to_string(i + 1) + ": sha256Hex es determinista");
    }
}


static void testEnumeracionExhaustivaCompleta() {
    std::cout << "\n-- Enumeracion exhaustiva sobre un alfabeto pequeno (correctitud logica) --\n";
    const std::string alfabetoChico = "ab";  // Sigma={a,b}, n=1..3 -> 2+4+8=14 candidatos

    
    fb::FBConfig config{alfabetoChico, 1, 3, std::string(64, '0')};  // hash imposible
    fb::FBResult r = fb::bruteForceSearch(config);

    long long esperado = 2 + 4 + 8;  // |Sigma|^1 + |Sigma|^2 + |Sigma|^3
    CHECK(!r.found, "con un hash objetivo imposible, no se declara ningun candidato como encontrado");
    CHECK_EQ(r.candidatesEvaluated, esperado,
             "se evaluan exactamente |Sigma|^1+|Sigma|^2+|Sigma|^3 candidatos (sin omisiones ni repeticiones)");

   
    std::set<std::string> vistos;
    std::vector<std::string> esperados;
    for (const std::string& base : {"a", "b"}) esperados.push_back(base);
    for (char c1 : {'a', 'b'}) for (char c2 : {'a', 'b'}) esperados.push_back(std::string() + c1 + c2);
    for (char c1 : {'a', 'b'}) for (char c2 : {'a', 'b'}) for (char c3 : {'a', 'b'})
        esperados.push_back(std::string() + c1 + c2 + c3);

    bool todasEncontrablesRapido = true;
    for (const auto& candidato : esperados) {
        fb::FBConfig cfg2{alfabetoChico, (int)candidato.size(), 3, fb::sha256Hex(candidato)};
        fb::FBResult r2 = fb::bruteForceSearch(cfg2);
        if (!r2.found || r2.password != candidato) { todasEncontrablesRapido = false; break; }
    }
    CHECK(todasEncontrablesRapido, "todo candidato de Sigma^n para n=1..3 es encontrable (cobertura completa)");
}

int main() {
    testVectoresSHA256();
    testInstanciaReferencia();
    testAtaqueDiccionario();
    testConsistenciaInstanciasEquipo();
    testEnumeracionExhaustivaCompleta();
    return resumenFinal("tests/test_fb.cpp");
}
