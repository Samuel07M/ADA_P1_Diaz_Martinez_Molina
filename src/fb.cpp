#include "fb.hpp"
#include "third_party/picosha2.h"
#include <chrono>
#include <fstream>

// ===========================================================================
// MODULO DE FUERZA BRUTA (FB) - Implementacion
// ===========================================================================

namespace fb {

// ---------------------------------------------------------------------
// sha256Hex
// Solo es un "envoltorio" sobre la libreria picosha2, para no tener
// que escribir su nombre largo en todo el codigo. Recibe un texto
// plano y devuelve su hash en formato hexadecimal (64 caracteres).
// ---------------------------------------------------------------------
std::string sha256Hex(const std::string& input) {
    std::string hashHex;
    picosha2::hash256_hex_string(input, hashHex);
    return hashHex;
}

// ---------------------------------------------------------------------
// nextCandidate (funcion interna, no se usa fuera de este archivo)
//
// Avanza una cadena al siguiente candidato, como si fuera un odometro
// de carro. Cada posicion de la cadena es un "digito" en base
// |alphabet|. Si el simbolo actual es el ultimo del alfabeto, se
// reinicia al primero y se "lleva 1" a la posicion de la izquierda
// (esto se llama acarreo, igual que sumar 1 en cualquier base).
//
// Devuelve true si YA SE RECORRIERON todas las combinaciones posibles
// de esa longitud (el acarreo se salio por la posicion 0).
//
// Por que esto no repite ni se salta ninguna cadena: es exactamente el
// mismo procedimiento que usamos para contar en base 10 (0,1,2,...,9,10,11...)
// pero en base |alphabet|. Contar consecutivamente nunca repite ni
// omite un numero, entonces tampoco lo hace con las cadenas.
// ---------------------------------------------------------------------
static bool nextCandidate(std::string& candidate, const std::string& alphabet) {
    int pos = (int)candidate.size() - 1;
    bool carry = true;

    while (pos >= 0 && carry) {
        int idx = (int)alphabet.find(candidate[pos]);

        if (idx < (int)alphabet.size() - 1) {
            // todavia hay un siguiente simbolo disponible en esta posicion
            candidate[pos] = alphabet[idx + 1];
            carry = false;
        } else {
            // era el ultimo simbolo: reiniciamos y pasamos el acarreo
            // a la posicion de la izquierda
            candidate[pos] = alphabet[0];
            pos--;
        }
    }

    return carry; // si sigue true, ya no quedan mas combinaciones de este largo
}

// ---------------------------------------------------------------------
// bruteForceSearch
// Recorre TODAS las cadenas posibles del alfabeto, desde longitud
// minLength hasta maxLength, calcula el hash de cada una y lo compara
// con el hash objetivo. Se detiene apenas encuentra una coincidencia.
// ---------------------------------------------------------------------
FBResult bruteForceSearch(const FBConfig& config) {
    auto startTime = std::chrono::high_resolution_clock::now();
    long long evaluated = 0;

    for (int n = config.minLength; n <= config.maxLength; n++) {
        // primer candidato de esta longitud: el simbolo mas "pequeño"
        // del alfabeto repetido n veces (ej: "aaaa")
        std::string candidate(n, config.alphabet[0]);
        bool exhausted = false;

        while (!exhausted) {
            evaluated++;
            std::string hash = sha256Hex(candidate);

            if (hash == config.targetHashHex) {
                auto endTime = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();
                return FBResult{true, candidate, evaluated, ms};
            }

            exhausted = nextCandidate(candidate, config.alphabet);
        }
    }

    // se probaron todas las longitudes del rango y no hubo coincidencia
    auto endTime = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return FBResult{false, "", evaluated, ms};
}

// ---------------------------------------------------------------------
// dictionaryAttack
// En vez de probar todas las combinaciones posibles, solo prueba las
// palabras que estan dentro de un archivo de texto (una por linea).
// Es mucho mas rapido, pero NO es exhaustivo: si la contraseña no
// esta en el archivo, este algoritmo nunca la va a encontrar.
// ---------------------------------------------------------------------
FBResult dictionaryAttack(const std::string& targetHashHex,
                           const std::string& dictionaryPath) {
    auto startTime = std::chrono::high_resolution_clock::now();
    long long evaluated = 0;

    std::ifstream file(dictionaryPath);
    std::string candidate;

    while (std::getline(file, candidate)) {
        if (candidate.empty()) continue; // saltamos lineas vacias
        evaluated++;

        std::string hash = sha256Hex(candidate);
        if (hash == targetHashHex) {
            auto endTime = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();
            return FBResult{true, candidate, evaluated, ms};
        }
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(endTime - startTime).count();
    return FBResult{false, "", evaluated, ms};
}

} // namespace fb