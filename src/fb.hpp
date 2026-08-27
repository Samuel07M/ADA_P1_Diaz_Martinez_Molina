#pragma once
#include <string>

// ===========================================================================
// MODULO DE FUERZA BRUTA (FB)
//
// Este archivo es la "interfaz" del modulo: aqui solo declaramos QUE hace
// cada funcion (su firma), no COMO lo hace. La implementacion real esta
// en fb.cpp. Esto nos permite que main.cpp use estas funciones sin
// preocuparse por los detalles internos.
// ===========================================================================

namespace fb {

// Guarda todo lo que necesita una busqueda por fuerza bruta:
// que simbolos usar, que tan larga puede ser la contraseña, y
// cual es el hash que estamos tratando de "romper".
struct FBConfig {
    std::string alphabet;      // simbolos permitidos, ej: "abcdefghijklmnopqrstuvwxyz"
    int minLength;             // longitud minima de contraseña a probar
    int maxLength;             // longitud maxima de contraseña a probar
    std::string targetHashHex; // hash SHA-256 objetivo, en hexadecimal
};

// Guarda el resultado de CUALQUIER busqueda (fuerza bruta o diccionario).
// Usamos la misma estructura para las dos, asi es mas facil comparlarlas.
struct FBResult {
    bool found;                     // true si se encontro la contraseña
    std::string password;           // la contraseña encontrada (vacia si no se encontro)
    long long candidatesEvaluated;  // cuantos candidatos se probaron en total
    double elapsedMs;               // tiempo que tomo la busqueda, en milisegundos
};

// Calcula el hash SHA-256 de un texto y lo devuelve como texto hexadecimal.
// Es un "envoltorio" (wrapper) sobre la libreria externa picosha2.
std::string sha256Hex(const std::string& input);

// Prueba TODAS las combinaciones posibles del alfabeto, desde longitud
// minLength hasta maxLength, hasta encontrar la contraseña o agotar
// el espacio de busqueda. Esto es Fuerza Bruta pura.
FBResult bruteForceSearch(const FBConfig& config);

// Prueba unicamente las palabras que estan dentro de un archivo de texto
// (una palabra por linea). Es mucho mas rapido que la fuerza bruta, pero
// NO garantiza encontrar la contraseña si esta no esta en el archivo.
FBResult dictionaryAttack(const std::string& targetHashHex,
                           const std::string& dictionaryPath);

} // namespace fb