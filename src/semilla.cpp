#include "semilla.hpp"

#include <algorithm>
#include <cctype>

namespace semilla {

namespace {

// Mapea un caracter UTF-8 de vocal acentuada / dieresis / enye a su
// version sin tilde en minuscula. Cubre el alfabeto espanol; para
// cualquier otro caracter no reconocido se ignora (no se agrega).
// Las tildes en UTF-8 ocupan 2 bytes (0xC3 0xA1 == 'a', etc.), por eso
// se procesa byte a byte con una tabla del segundo byte.
char desacentuar(unsigned char b1, unsigned char b2, bool& esDosBytes) {
    esDosBytes = (b1 == 0xC3);
    if (!esDosBytes) return 0;
    switch (b2) {
        case 0xA1: case 0x81: return 'a';  // a con tilde (min/mayus)
        case 0xA9: case 0x89: return 'e';
        case 0xAD: case 0x8D: return 'i';
        case 0xB3: case 0x93: return 'o';
        case 0xBA: case 0x9A: return 'u';
        case 0xBC: case 0x9C: return 'u';  // u con dieresis
        case 0xB1: case 0x91: return 'n';  // enye
        default: return 0;
    }
}

}  // namespace

std::string normalizarApellido(const std::string& apellido) {
    std::string out;
    out.reserve(apellido.size());
    for (size_t i = 0; i < apellido.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(apellido[i]);
        if (c == ' ' || c == '-' || c == '\'') continue;  // sin espacios
        if (c < 0x80) {
            if (std::isalpha(c)) out += static_cast<char>(std::tolower(c));
            continue;
        }
        // Posible secuencia UTF-8 de 2 bytes (tildes/enye/dieresis).
        if (i + 1 < apellido.size()) {
            bool dosBytes = false;
            char plano = desacentuar(c, static_cast<unsigned char>(apellido[i + 1]), dosBytes);
            if (dosBytes) {
                if (plano) out += plano;
                ++i;  // consumir el segundo byte de la secuencia UTF-8
                continue;
            }
        }
        // Byte no reconocido (otro rango UTF-8): se ignora.
    }
    return out;
}

uint64_t calcularSemilla(std::vector<std::string> apellidos) {
    for (auto& a : apellidos) a = normalizarApellido(a);
    std::sort(apellidos.begin(), apellidos.end());

    std::string concatenado;
    for (const auto& a : apellidos) concatenado += a;

    uint64_t suma = 0;
    for (unsigned char c : concatenado) suma += c;

    return suma % 100000ULL;
}

uint64_t siguienteLCG(uint64_t x) {
    const uint64_t MULT = 1103515245ULL;
    const uint64_t INC = 12345ULL;
    const uint64_t MOD = 2147483648ULL;  // 2^31
    return (MULT * x + INC) % MOD;
}

std::vector<std::string> generarContrasenasEquipo(
    uint64_t semillaInicial, const std::vector<std::string>& alfabetos,
    const std::vector<int>& longitudes) {
    std::vector<std::string> resultado;
    resultado.reserve(alfabetos.size());
    uint64_t x = semillaInicial;  // x_0 = semilla

    for (size_t inst = 0; inst < longitudes.size(); ++inst) {
        std::string pwd;
        const std::string& alpha = alfabetos[inst];
        pwd.reserve(longitudes[inst]);

        for (int pos = 0; pos < longitudes[inst]; ++pos) {
            int idx = static_cast<int>(x % alpha.size());
            pwd += alpha[idx];
            x = siguienteLCG(x);
        }
        resultado.push_back(pwd);
    }

    return resultado;
}

ParametrosPoliticaBT derivarPoliticaBT(uint64_t semillaEquipo) {
    ParametrosPoliticaBT p;
    p.minLower = 2 + static_cast<int>(semillaEquipo % 3);
    p.minUpper = 1 + static_cast<int>(semillaEquipo % 2);
    p.minDigit = 1 + static_cast<int>(semillaEquipo % 3);
    p.minSymbol = 1;
    return p;
}

}  // namespace semilla
