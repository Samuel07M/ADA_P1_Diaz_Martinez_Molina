
#include "../src/semilla.hpp"
#include "../src/fb_fuerzabruta.hpp"
#include <algorithm>
#include <iostream>
#include <vector>
#include <string>

int main() {
    const std::vector<std::string> apellidos = {"Martinez", "Molina", "Diaz"};
    const uint64_t SEMILLA_ESPERADA = 1938;

    std::cout << "=== Verificacion de semilla del equipo ===\n\n";

    std::cout << "1) Apellidos originales: ";
    for (size_t i = 0; i < apellidos.size(); i++) {
        if (i) std::cout << ", ";
        std::cout << apellidos[i];
    }
    std::cout << "\n";

    std::vector<std::string> normalizados;
    for (const auto& a : apellidos) normalizados.push_back(semilla::normalizarApellido(a));
    std::cout << "2) Normalizados (minusculas, sin tildes/espacios): ";
    for (size_t i = 0; i < normalizados.size(); i++) {
        if (i) std::cout << ", ";
        std::cout << normalizados[i];
    }
    std::cout << "\n";

    std::vector<std::string> ordenados = normalizados;
    std::sort(ordenados.begin(), ordenados.end());
    std::cout << "3) Orden alfabetico: ";
    for (size_t i = 0; i < ordenados.size(); i++) {
        if (i) std::cout << ", ";
        std::cout << ordenados[i];
    }
    std::cout << "\n";

    std::string concatenado;
    for (const auto& a : ordenados) concatenado += a;
    std::cout << "4) Concatenacion: \"" << concatenado << "\" (" << concatenado.size()
              << " caracteres)\n";

    long long sumaAscii = 0;
    for (unsigned char c : concatenado) sumaAscii += c;
    std::cout << "5) Suma de codigos ASCII: " << sumaAscii << "\n";

    uint64_t semillaCalculada = semilla::calcularSemilla(apellidos);
    std::cout << "6) Semilla = (suma ASCII) mod 100000 = " << semillaCalculada << "\n\n";

    bool ok = true;

    std::cout << "--- Verificacion 1: semilla declarada por el equipo ---\n";
    if (semillaCalculada == SEMILLA_ESPERADA) {
        std::cout << "[OK]    Semilla calculada (" << semillaCalculada
                  << ") == semilla declarada (" << SEMILLA_ESPERADA << ")\n\n";
    } else {
        std::cout << "[FALLO] Semilla calculada (" << semillaCalculada
                  << ") != semilla declarada (" << SEMILLA_ESPERADA << ")\n\n";
        ok = false;
    }

    std::cout << "--- Verificacion 2: instancia de referencia comun al curso (FB) ---\n";
    const std::string REF_PWD = "abc12";
    const std::string REF_HASH_ESPERADO =
        "8d51feb34e3e69f6fa6dffc577e2c60490cf9a7fcd835f9f6af1505b71d74773";
    std::string refHashCalculado = fb::sha256Hex(REF_PWD);
    std::cout << "  sha256(\"" << REF_PWD << "\") = " << refHashCalculado << "\n";
    std::cout << "  publicado en el enunciado    = " << REF_HASH_ESPERADO << "\n";
    if (refHashCalculado == REF_HASH_ESPERADO) {
        std::cout << "[OK]    El hash calculado coincide con el publicado (verificado tambien con "
                     "`echo -n abc12 | sha256sum`).\n\n";
    } else {
        std::cout << "[FALLO] El hash calculado NO coincide con el publicado.\n\n";
        ok = false;
    }

    std::cout << "--- Verificacion 3: parametros de politica BT derivados de la semilla ---\n";
    semilla::ParametrosPoliticaBT p = semilla::derivarPoliticaBT(semillaCalculada);
    std::cout << "  minLower  = 2 + (" << semillaCalculada << " mod 3) = " << p.minLower << "\n";
    std::cout << "  minUpper  = 1 + (" << semillaCalculada << " mod 2) = " << p.minUpper << "\n";
    std::cout << "  minDigit  = 1 + (" << semillaCalculada << " mod 3) = " << p.minDigit << "\n";
    std::cout << "  minSymbol = 1\n";
    int suma = p.minLower + p.minUpper + p.minDigit + p.minSymbol;
    std::cout << "  suma = " << suma << " (debe ser <= n=8)\n";
    if (suma <= 8) {
        std::cout << "[OK]    suma <= 8, no hace falta reducir minLower.\n\n";
    } else {
        std::cout << "[FALLO] suma > 8, main_bt.cpp deberia reducir minLower (ver salvaguarda).\n\n";
        ok = false;
    }

    std::cout << (ok ? "RESULTADO: TODAS LAS VERIFICACIONES PASARON.\n"
                      : "RESULTADO: HUBO AL MENOS UNA VERIFICACION FALLIDA.\n");

    return ok ? 0 : 1;
}
